param([string]$Output = 'out/dependencies', [string]$SevenZip = '7z')
$ErrorActionPreference = 'Stop'
$sourceDirectory = Join-Path $Output 'sources'
$noticeDirectory = Join-Path $Output 'notices'
New-Item -ItemType Directory -Force -Path $sourceDirectory, $noticeDirectory | Out-Null
$modules = @('qtbase', 'qtdeclarative', 'qtmultimedia', 'qtsvg', 'qtshadertools', 'qtwayland', 'qttranslations')
$manifest = @()
foreach ($module in $modules) {
    $name = "$module-everywhere-src-6.11.2.tar.xz"
    $url = "https://download.qt.io/official_releases/qt/6.11/6.11.2/submodules/$name"
    $archive = Join-Path $sourceDirectory $name
    if (-not (Test-Path -LiteralPath $archive)) {
        & curl.exe --fail --location --silent --show-error --retry 3 --output $archive $url
        if ($LASTEXITCODE -ne 0) { throw "Could not download $name" }
    }
    $checksum = (Invoke-WebRequest -UseBasicParsing -Uri ($url + '.sha256')).Content
    if ($checksum -is [byte[]]) { $checksum = [System.Text.Encoding]::UTF8.GetString($checksum) }
    $expected = [regex]::Match($checksum, '[a-fA-F0-9]{64}').Value
    $actual = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $expected.ToLowerInvariant()) { throw "Checksum failed for $name" }
    $manifest += [ordered]@{ file = $name; url = $url; sha256 = $actual }
    & $SevenZip x $archive "-o$Output" -y -bso0 -bsp0
    if ($LASTEXITCODE -ne 0) { throw "Could not expand $name" }
    $expanded = Join-Path $Output $name.Substring(0, $name.Length - 3)
    $members = @(& tar -tf $expanded | Where-Object {
        $_ -match '/LICENSES/[^/]+$|/qt_attribution.json$|/(COPYING|LICENSE|LICENCE|Copyright|copyright)([^/]*)$'
    })
    $memberFile = Join-Path $Output 'members.txt'
    [System.IO.File]::WriteAllLines([System.IO.Path]::GetFullPath($memberFile), $members)
    & tar -xf $expanded -C $noticeDirectory -T $memberFile
    if ($LASTEXITCODE -ne 0) { throw "Could not extract notices for $module" }
    Write-Output "Verified $module 6.11.2 source and notices"
    Remove-Item -LiteralPath $expanded
}
$ffmpegName = 'ffmpeg-7.1.5.tar.xz'
$ffmpegUrl = "https://ffmpeg.org/releases/$ffmpegName"
$ffmpegArchive = Join-Path $sourceDirectory $ffmpegName
if (-not (Test-Path -LiteralPath $ffmpegArchive)) {
    & curl.exe --fail --location --silent --show-error --retry 3 --output $ffmpegArchive $ffmpegUrl
    if ($LASTEXITCODE -ne 0) { throw 'Could not download FFmpeg source' }
}
$manifest += [ordered]@{
    file = $ffmpegName
    url = $ffmpegUrl
    sha256 = (Get-FileHash -LiteralPath $ffmpegArchive -Algorithm SHA256).Hash.ToLowerInvariant()
}
& $SevenZip x $ffmpegArchive "-o$Output" -y -bso0 -bsp0
if ($LASTEXITCODE -ne 0) { throw 'Could not expand FFmpeg source' }
$expanded = Join-Path $Output 'ffmpeg-7.1.5.tar'
$members = @(& tar -tf $expanded | Where-Object { $_ -match '/(COPYING[^/]*|LICENSE.md)$' })
[System.IO.File]::WriteAllLines([System.IO.Path]::GetFullPath((Join-Path $Output 'members.txt')), $members)
& tar -xf $expanded -C $noticeDirectory -T (Join-Path $Output 'members.txt')
if ($LASTEXITCODE -ne 0) { throw 'Could not extract FFmpeg notices' }
Remove-Item -LiteralPath $expanded
$runtimeSources = @(
    @{ Name = 'appimage-runtime-75849dce.tar.gz'; Url = 'https://codeload.github.com/AppImage/type2-runtime/tar.gz/75849dce7cc37e4319b633df1f116ca895c71a12' },
    @{ Name = 'fuse-3.15.0.tar.xz'; Url = 'https://github.com/libfuse/libfuse/releases/download/fuse-3.15.0/fuse-3.15.0.tar.xz'; Hash = '70589cfd5e1cff7ccd6ac91c86c01be340b227285c5e200baa284e401eea2ca0' },
    @{ Name = 'squashfuse-0.5.2.tar.gz'; Url = 'https://codeload.github.com/vasi/squashfuse/tar.gz/refs/tags/0.5.2'; Hash = 'db0238c5981dabbd80ee09ae15387f390091668ca060a7bc38047912491443d3' },
    @{ Name = 'musl-1.2.5.tar.gz'; Url = 'https://musl.libc.org/releases/musl-1.2.5.tar.gz' },
    @{ Name = 'zstd-1.5.6.tar.gz'; Url = 'https://codeload.github.com/facebook/zstd/tar.gz/refs/tags/v1.5.6' },
    @{ Name = 'zlib-1.3.1.tar.gz'; Url = 'https://zlib.net/fossils/zlib-1.3.1.tar.gz' }
)
foreach ($source in $runtimeSources) {
    $archive = Join-Path $sourceDirectory $source.Name
    if (-not (Test-Path -LiteralPath $archive)) {
        & curl.exe --fail --location --silent --show-error --retry 3 --output $archive $source.Url
        if ($LASTEXITCODE -ne 0) { throw "Could not download $($source.Name)" }
    }
    $actual = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($source.Hash -and $actual -ne $source.Hash) { throw "Checksum failed for $($source.Name)" }
    $manifest += [ordered]@{ file = $source.Name; url = $source.Url; sha256 = $actual }
    & $SevenZip x $archive "-o$Output" -y -bso0 -bsp0
    if ($LASTEXITCODE -ne 0) { throw "Could not expand $($source.Name)" }
    $expanded = Join-Path $Output $source.Name.Substring(0, $source.Name.Length - 3)
    $members = @(& tar -tf $expanded | Where-Object { $_ -match '/(COPYING[^/]*|LICENSE[^/]*|LICENCE[^/]*|COPYRIGHT|copyright)$' })
    [System.IO.File]::WriteAllLines([System.IO.Path]::GetFullPath((Join-Path $Output 'members.txt')), $members)
    & tar -xf $expanded -C $noticeDirectory -T (Join-Path $Output 'members.txt')
    if ($LASTEXITCODE -ne 0) { throw "Could not extract notices for $($source.Name)" }
    Remove-Item -LiteralPath $expanded
    Write-Output "Included $($source.Name) source and notices"
}
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $sourceDirectory 'manifest.json') -Encoding utf8
Copy-Item -LiteralPath (Join-Path $sourceDirectory 'manifest.json') -Destination (Join-Path $noticeDirectory 'source-manifest.json')
New-Item -ItemType Directory -Force -Path licenses | Out-Null
Compress-Archive -Path (Join-Path $noticeDirectory '*') -DestinationPath licenses/dependency-notices.zip -Force
Compress-Archive -Path (Join-Path $sourceDirectory '*') -DestinationPath (Join-Path $Output 'BetterPiP-0.1.0-dependency-sources.zip') -Force
