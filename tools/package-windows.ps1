param([string]$Dist = 'dist', [string]$Output = 'out/release', [string]$Version = '0.1.0')
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath $Dist).Path
New-Item -ItemType Directory -Force -Path $Output | Out-Null
$outputDirectory = (Resolve-Path -LiteralPath $Output).Path
$savedEnvironment = @{}
foreach ($name in @('PATH', 'QT_PLUGIN_PATH', 'QML2_IMPORT_PATH', 'QML_IMPORT_PATH', 'QT_QPA_PLATFORM', 'QT_QUICK_BACKEND')) {
    $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
}
try {
    $env:PATH = "$env:SystemRoot\System32;$env:SystemRoot"
    $env:QT_PLUGIN_PATH = $null
    $env:QML2_IMPORT_PATH = $null
    $env:QML_IMPORT_PATH = $null
    $env:QT_QPA_PLATFORM = 'offscreen'
    $env:QT_QUICK_BACKEND = 'software'
    foreach ($runtime in @('msvcp140.dll', 'vcruntime140.dll', 'vcruntime140_1.dll')) {
        if (-not (Test-Path -LiteralPath (Join-Path $root "bin/$runtime"))) { throw "Missing runtime: $runtime" }
    }
    $process = Start-Process -FilePath (Join-Path $root 'bin/better-pip.exe') -ArgumentList '--smoke-test' -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit(20000)) {
        $process.Kill()
        throw 'Packaged application startup timed out.'
    }
    if ($process.ExitCode -ne 0) { throw "Packaged application failed: $($process.ExitCode)" }
} finally {
    foreach ($name in $savedEnvironment.Keys) {
        [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name], 'Process')
    }
}
$uninstall = @()
foreach ($file in Get-ChildItem -LiteralPath $root -Recurse -File) {
    $relative = [System.IO.Path]::GetRelativePath($root, $file.FullName).Replace('$', '$$')
    $uninstall += 'Delete "$INSTDIR\' + $relative + '"'
}
foreach ($directory in Get-ChildItem -LiteralPath $root -Recurse -Directory | Sort-Object { $_.FullName.Length } -Descending) {
    $relative = [System.IO.Path]::GetRelativePath($root, $directory.FullName).Replace('$', '$$')
    $uninstall += 'RMDir "$INSTDIR\' + $relative + '"'
}
$uninstallFile = Join-Path $outputDirectory 'uninstall-files.nsh'
[System.IO.File]::WriteAllLines($uninstallFile, $uninstall, [System.Text.UTF8Encoding]::new($false))
$compiler = Get-Command makensis.exe -ErrorAction SilentlyContinue
if (-not $compiler) {
    $candidate = Join-Path ${env:ProgramFiles(x86)} 'NSIS/makensis.exe'
    if (Test-Path -LiteralPath $candidate) { $compiler = Get-Item -LiteralPath $candidate }
}
if (-not $compiler) { throw 'Install NSIS 3 to create the Windows installer.' }
$compilerPath = if ($compiler.Source) { $compiler.Source } else { $compiler.FullName }
$installer = Join-Path $outputDirectory "BetterPiP-$Version-windows-x64-setup.exe"
& $compilerPath /V2 "/DVERSION=$Version" "/DDIST_DIR=$root" "/DOUTPUT_FILE=$installer" "/DUNINSTALL_FILES=$uninstallFile" packaging/windows/installer.nsi
if ($LASTEXITCODE -ne 0) { throw 'Installer creation failed.' }
Compress-Archive -Path (Join-Path $root '*') -DestinationPath (Join-Path $outputDirectory "BetterPiP-$Version-windows-x64.zip") -Force
Get-ChildItem -LiteralPath $outputDirectory -File | Where-Object { $_.Extension -in '.exe', '.zip' } | ForEach-Object {
    ((Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() + '  ' + $_.Name)
} | Set-Content -LiteralPath (Join-Path $outputDirectory 'SHA256SUMS-windows.txt') -Encoding ascii
