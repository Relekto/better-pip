param([Parameter(Mandatory = $true)][string]$Destination)
$ErrorActionPreference = 'Stop'
$repository = 'https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_6112/qt6_6112_msvc2022_64/'
[xml]$updates = (Invoke-WebRequest -UseBasicParsing -Uri ($repository + 'Updates.xml')).Content
$packages = $updates.Updates.PackageUpdate | Where-Object {
    $_.Name -in @('qt.qt6.6112.win64_msvc2022_64',
                 'qt.qt6.6112.addons.qtmultimedia.win64_msvc2022_64',
                 'qt.qt6.6112.addons.qtshadertools.win64_msvc2022_64')
}
if ($packages.Count -ne 3) { throw 'The pinned Qt package manifest is incomplete.' }
$cache = Join-Path $Destination 'archives'
New-Item -ItemType Directory -Force -Path $cache | Out-Null
foreach ($package in $packages) {
    foreach ($archive in ($package.DownloadableArchives -split ', ')) {
        if ($archive -notmatch '^(qtbase|qtdeclarative|qtsvg|qtmultimedia|qtshadertools)-') { continue }
        $url = $repository + $package.Name + '/' + $package.Version + $archive
        $file = Join-Path $cache $archive
        & curl.exe --fail --location --silent --show-error --retry 3 --output $file $url
        if ($LASTEXITCODE -ne 0) { throw "Download failed: $archive" }
        $checksum = (Invoke-WebRequest -UseBasicParsing -Uri ($url + '.sha1')).Content
        if ($checksum -is [byte[]]) { $checksum = [System.Text.Encoding]::UTF8.GetString($checksum) }
        if ((Get-FileHash -LiteralPath $file -Algorithm SHA1).Hash -ne $checksum.Trim().Split(' ')[0]) {
            throw "Archive integrity check failed: $archive"
        }
        & 7z x $file "-o$Destination" -y -bso0 -bsp0
        if ($LASTEXITCODE -ne 0) { throw "Extraction failed: $archive" }
    }
}
[System.IO.File]::WriteAllText((Join-Path $Destination 'bin/qt.conf'), "[Paths]`nPrefix=..`n")
if ($env:GITHUB_PATH) {
    (Join-Path $Destination 'bin') | Out-File -FilePath $env:GITHUB_PATH -Append -Encoding utf8
    "CMAKE_PREFIX_PATH=$Destination" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
    "QT_ROOT_DIR=$Destination" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
}
