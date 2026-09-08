#include "PortalCapture.h"
#include "PortalRequest.h"
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusUnixFileDescriptor>
#include <QPointer>
#include <QUuid>
#include <QVideoFrameFormat>
#include <pipewire/pipewire.h>
#include <spa/param/video/format-utils.h>
#include <array>
#include <cstring>
#include <mutex>
#include <unistd.h>

namespace pip {
namespace {
const QString service = QStringLiteral("org.freedesktop.portal.Desktop");
const QString desktop = QStringLiteral("/org/freedesktop/portal/desktop");
const QString screenCast = QStringLiteral("org.freedesktop.portal.ScreenCast");
QString objectPath(const QVariant& value) {
    return value.canConvert<QDBusObjectPath>() ? value.value<QDBusObjectPath>().path() : value.toString();
}
}
struct PortalCapture::Private {
    explicit Private(PortalCapture& owner) : owner(owner) {}
    PortalCapture& owner;
    QPointer<PortalRequest> request;
    QString session;
    pw_thread_loop* loop{nullptr};
    pw_context* context{nullptr};
    pw_core* core{nullptr};
    pw_stream* stream{nullptr};
    spa_hook listener{};
    spa_video_info_raw format{};
    std::mutex mutex;
    QVideoFrame latest;
    bool scheduled{false};
    quint64 generation{0};

    void error(const QString& message) {
        owner.stop();
        emit owner.failed(message);
    }
    void select() {
        const QVariantMap options{{QStringLiteral("types"), uint(2)},
                                  {QStringLiteral("multiple"), false},
                                  {QStringLiteral("cursor_mode"), uint(1)}};
        request = PortalRequest::call(screenCast, QStringLiteral("SelectSources"),
                        {QVariant::fromValue(QDBusObjectPath(session))}, options, &owner);
        QObject::connect(request, &PortalRequest::finished, &owner,
            [this](uint code, const QVariantMap&, const QString& message) {
                request.clear();
                if (code != 0) { error(message.isEmpty() ? QStringLiteral("Window sharing is unavailable.") : message); return; }
                begin();
            });
    }
    void begin() {
        request = PortalRequest::call(screenCast, QStringLiteral("Start"),
                         {QVariant::fromValue(QDBusObjectPath(session)), QString{}}, {}, &owner);
        QObject::connect(request, &PortalRequest::finished, &owner,
            [this](uint code, const QVariantMap& results, const QString& message) {
                request.clear();
                if (code != 0) { error(message.isEmpty() ? QStringLiteral("Window sharing could not start.") : message); return; }
                const auto argument = results.value(QStringLiteral("streams")).value<QDBusArgument>();
                uint node = PW_ID_ANY;
                QVariantMap properties;
                argument.beginArray();
                if (!argument.atEnd()) {
                    argument.beginStructure();
                    argument >> node >> properties;
                    argument.endStructure();
                }
                argument.endArray();
                if (node == PW_ID_ANY) { error(QStringLiteral("No window stream was returned.")); return; }
                openRemote(node, properties.value(QStringLiteral("pipewire-serial")).toString());
            });
    }
    void openRemote(uint node, const QString& serial) {
        auto message = QDBusMessage::createMethodCall(service, desktop, screenCast,
                                                     QStringLiteral("OpenPipeWireRemote"));
        message.setArguments({QVariant::fromValue(QDBusObjectPath(session)), QVariantMap{}});
        const auto epoch = generation;
        auto* watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), &owner);
        QObject::connect(watcher, &QDBusPendingCallWatcher::finished, &owner,
            [this, node, serial, epoch](QDBusPendingCallWatcher* completed) {
                QDBusPendingReply<QDBusUnixFileDescriptor> reply = *completed;
                completed->deleteLater();
                if (epoch != generation) return;
                if (reply.isError()) { error(reply.error().message()); return; }
                const int fd = dup(reply.value().fileDescriptor());
                if (fd < 0) { error(QStringLiteral("Could not open the shared window stream.")); return; }
                connectStream(fd, node, serial);
            });
    }
    static void stateChanged(void* data, pw_stream_state, pw_stream_state state, const char* message) {
        auto& self = *static_cast<Private*>(data);
        if (state != PW_STREAM_STATE_ERROR) return;
        const auto text = QString::fromUtf8(message ? message : "The shared stream stopped.");
        const auto epoch = self.generation;
        QMetaObject::invokeMethod(&self.owner, [&self, text, epoch] {
            if (epoch == self.generation) self.error(text);
        }, Qt::QueuedConnection);
    }
    static void parameterChanged(void* data, uint32_t id, const spa_pod* parameter) {
        auto& self = *static_cast<Private*>(data);
        if (id == SPA_PARAM_Format && parameter)
            spa_format_video_raw_parse(parameter, &self.format);
    }
    static void process(void* data) {
        auto& self = *static_cast<Private*>(data);
        auto* buffer = pw_stream_dequeue_buffer(self.stream);
        if (!buffer) return;
        struct ReturnBuffer {
            pw_stream* stream;
            pw_buffer* buffer;
            ~ReturnBuffer() { pw_stream_queue_buffer(stream, buffer); }
        } returnBuffer{self.stream, buffer};
        if (!buffer->buffer || buffer->buffer->n_datas == 0) return;
        const auto& plane = buffer->buffer->datas[0];
        const auto width = self.format.size.width;
        const auto height = self.format.size.height;
        if (!plane.data || !plane.chunk || width == 0 || height == 0 || width > 8192 || height > 8192) return;
        const auto stride = plane.chunk->stride;
        const auto rowBytes = static_cast<quint64>(width) * 4;
        if (stride < 0 || static_cast<quint64>(stride) < rowBytes) return;
        const auto required = static_cast<quint64>(height - 1) * static_cast<quint64>(stride) + rowBytes;
        if (plane.chunk->offset > plane.maxsize || required > plane.maxsize - plane.chunk->offset ||
            required > plane.chunk->size) return;
        QVideoFrameFormat::PixelFormat pixelFormat;
        switch (self.format.format) {
        case SPA_VIDEO_FORMAT_BGRA: pixelFormat = QVideoFrameFormat::Format_BGRA8888; break;
        case SPA_VIDEO_FORMAT_BGRx: pixelFormat = QVideoFrameFormat::Format_BGRX8888; break;
        case SPA_VIDEO_FORMAT_RGBA: pixelFormat = QVideoFrameFormat::Format_RGBA8888; break;
        case SPA_VIDEO_FORMAT_RGBx: pixelFormat = QVideoFrameFormat::Format_RGBX8888; break;
        default: return;
        }
        QVideoFrame frame(QVideoFrameFormat(QSize(static_cast<int>(width), static_cast<int>(height)), pixelFormat));
        if (!frame.map(QVideoFrame::WriteOnly)) return;
        const auto* source = static_cast<const unsigned char*>(plane.data) + plane.chunk->offset;
        for (uint32_t row = 0; row < height; ++row)
            std::memcpy(frame.bits(0) + static_cast<qsizetype>(row) * frame.bytesPerLine(0),
                        source + static_cast<std::size_t>(row) * static_cast<std::size_t>(stride),
                        static_cast<std::size_t>(rowBytes));
        frame.unmap();
        std::lock_guard guard(self.mutex);
        self.latest = std::move(frame);
        if (self.scheduled) return;
        self.scheduled = true;
        const auto epoch = self.generation;
        QMetaObject::invokeMethod(&self.owner, [&self, epoch] {
            QVideoFrame frame;
            {
                std::lock_guard guard(self.mutex);
                if (epoch != self.generation) return;
                self.scheduled = false;
                frame = std::exchange(self.latest, {});
            }
            if (frame.isValid()) emit self.owner.frameReady(frame);
        }, Qt::QueuedConnection);
    }
    void connectStream(int fd, uint node, const QString& serial) {
        static std::once_flag initialized;
        std::call_once(initialized, [] { pw_init(nullptr, nullptr); });
        loop = pw_thread_loop_new("better-pip-capture", nullptr);
        if (!loop) { close(fd); error(QStringLiteral("Could not create the capture loop.")); return; }
        context = pw_context_new(pw_thread_loop_get_loop(loop), nullptr, 0);
        if (!context) { close(fd); error(QStringLiteral("Could not create the capture context.")); return; }
        core = pw_context_connect_fd(context, fd, nullptr, 0);
        if (!core) { error(QStringLiteral("Could not connect to PipeWire.")); return; }
        auto* properties = pw_properties_new(PW_KEY_MEDIA_TYPE, "Video", PW_KEY_MEDIA_CATEGORY, "Capture",
                                              PW_KEY_MEDIA_ROLE, "Screen", nullptr);
        if (!serial.isEmpty()) pw_properties_set(properties, PW_KEY_TARGET_OBJECT, serial.toUtf8().constData());
        stream = pw_stream_new(core, "Better PiP", properties);
        if (!stream) { error(QStringLiteral("Could not create the window stream.")); return; }
        static const pw_stream_events events = [] {
            pw_stream_events value{};
            value.version = PW_VERSION_STREAM_EVENTS;
            value.state_changed = &Private::stateChanged;
            value.param_changed = &Private::parameterChanged;
            value.process = &Private::process;
            return value;
        }();
        pw_stream_add_listener(stream, &listener, &events, this);
        alignas(8) std::array<uint8_t, 512> storage{};
        spa_pod_builder builder = SPA_POD_BUILDER_INIT(storage.data(), static_cast<uint32_t>(storage.size()));
        const spa_pod* parameter = static_cast<spa_pod*>(spa_pod_builder_add_object(&builder,
            SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
            SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video),
            SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
            SPA_FORMAT_VIDEO_format, SPA_POD_CHOICE_ENUM_Id(4, SPA_VIDEO_FORMAT_BGRA,
                SPA_VIDEO_FORMAT_BGRx, SPA_VIDEO_FORMAT_RGBA, SPA_VIDEO_FORMAT_RGBx)));
        const auto flags = static_cast<pw_stream_flags>(PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS);
        if (pw_stream_connect(stream, PW_DIRECTION_INPUT, serial.isEmpty() ? node : PW_ID_ANY,
                              flags, &parameter, 1) < 0 || pw_thread_loop_start(loop) < 0)
            error(QStringLiteral("The desktop could not provide a compatible window stream."));
    }
};
PortalCapture::PortalCapture(QObject* parent) : QObject(parent), d_(std::make_unique<Private>(*this)) {}
PortalCapture::~PortalCapture() { stop(); }
void PortalCapture::start() {
    stop();
    const QVariantMap options{{QStringLiteral("session_handle_token"),
        QStringLiteral("betterpip_") + QUuid::createUuid().toString(QUuid::Id128)}};
    d_->request = PortalRequest::call(screenCast, QStringLiteral("CreateSession"), {}, options, this);
    connect(d_->request, &PortalRequest::finished, this,
        [this](uint code, const QVariantMap& results, const QString& message) {
            d_->request.clear();
            if (code != 0) { d_->error(message.isEmpty() ? QStringLiteral("Window sharing is unavailable.") : message); return; }
            d_->session = objectPath(results.value(QStringLiteral("session_handle")));
            if (d_->session.isEmpty()) { d_->error(QStringLiteral("The desktop did not create a sharing session.")); return; }
            QDBusConnection::sessionBus().connect(service, d_->session,
                QStringLiteral("org.freedesktop.portal.Session"), QStringLiteral("Closed"),
                this, SLOT(sessionClosed()));
            d_->select();
        });
}
void PortalCapture::stop() {
    if (d_->request) {
        d_->request->cancel();
        d_->request->deleteLater();
        d_->request.clear();
    }
    if (d_->loop) pw_thread_loop_stop(d_->loop);
    {
        std::lock_guard guard(d_->mutex);
        ++d_->generation;
        d_->latest = {};
        d_->scheduled = false;
    }
    if (d_->stream) { pw_stream_destroy(d_->stream); d_->stream = nullptr; }
    if (d_->core) { pw_core_disconnect(d_->core); d_->core = nullptr; }
    if (d_->context) { pw_context_destroy(d_->context); d_->context = nullptr; }
    if (d_->loop) { pw_thread_loop_destroy(d_->loop); d_->loop = nullptr; }
    d_->format = {};
    if (!d_->session.isEmpty()) {
        QDBusConnection::sessionBus().disconnect(service, d_->session,
            QStringLiteral("org.freedesktop.portal.Session"), QStringLiteral("Closed"),
            this, SLOT(sessionClosed()));
        const auto message = QDBusMessage::createMethodCall(service, d_->session,
            QStringLiteral("org.freedesktop.portal.Session"), QStringLiteral("Close"));
        d_->session.clear();
        QDBusConnection::sessionBus().asyncCall(message);
    }
}
void PortalCapture::sessionClosed() { d_->error(QStringLiteral("Window sharing ended in your desktop.")); }
}
