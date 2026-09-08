param([Parameter(Mandatory = $true)][string]$Destination)
$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
$archive = Join-Path $Destination 'nsis-3.11.zip'
& curl.exe --fail --location --silent --show-error --connect-timeout 15 --max-time 120 --retry 2 --output $archive 'https://downloads.sourceforge.net/project/nsis/NSIS%203/3.11/nsis-3.11.zip'
if ($LASTEXITCODE -ne 0) { throw 'Could not download NSIS 3.11.' }
$expected = 'c7d27f780ddb6cffb4730138cd1591e841f4b7edb155856901cdf5f214394fa1'
if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) {
    throw 'NSIS archive integrity check failed.'
}
& 7z x $archive "-o$Destination" -y -bso0 -bsp0
if ($LASTEXITCODE -ne 0) { throw 'Could not extract NSIS.' }
$env:PATH = (Join-Path $Destination 'nsis-3.11') + ';' + $env:PATH
