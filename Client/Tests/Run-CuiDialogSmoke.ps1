[CmdletBinding()]
param(
    [string]$ResourceRoot = $env:MIR3_RESOURCE_ROOT,
    [string]$BuildDirectory,
    [switch]$Screenshots,
    [switch]$Preview,
    [Alias('ActionsCsv')]
    [string]$ScenarioCsv = (Join-Path $PSScriptRoot 'CuiDialogScenarios.csv')
)

$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
if (!$ResourceRoot) { $ResourceRoot = (Get-Location).Path }
$ResourceRoot = (Resolve-Path -LiteralPath $ResourceRoot).Path
foreach ($asset in @('Data/GameInter.wil', 'Data/Interface1c.wil', 'CmList.dat')) {
    if (!(Test-Path -LiteralPath (Join-Path $ResourceRoot $asset) -PathType Leaf)) {
        throw "找不到 $asset。请通过 -ResourceRoot 指定原版客户端的资源目录。"
    }
}
if (!$BuildDirectory) { $BuildDirectory = Join-Path $repo 'build-cui' }
$BuildDirectory = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($BuildDirectory)
$ScenarioCsv = (Resolve-Path -LiteralPath $ScenarioCsv).Path
if (!(Test-Path -LiteralPath (Join-Path $BuildDirectory 'CMakeCache.txt'))) {
    & cmake -S $repo -B $BuildDirectory -A Win32
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
& cmake --build $BuildDirectory --config Release --target Mir3CuiDialogSmoke
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$exe = Join-Path $BuildDirectory 'Release/Mir3CuiDialogSmoke.exe'
if (!(Test-Path -LiteralPath $exe -PathType Leaf)) {
    $exe = Join-Path $BuildDirectory 'Mir3CuiDialogSmoke.exe'
}
$run = Join-Path $BuildDirectory ('dialog-smoke/' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '-' + $PID)
New-Item -ItemType Directory -Path $run -Force | Out-Null
$smokeArgs = @($ResourceRoot,$run,$ScenarioCsv,'--defaults',(Join-Path $PSScriptRoot 'CuiDialogActions.csv'))
if ($Screenshots) { $smokeArgs += '--screenshots' }
if ($Preview) { $smokeArgs += '--preview' }
Write-Host "回放输出 $run"
& $exe @smokeArgs 2>&1 | Tee-Object -FilePath (Join-Path $run 'smoke.log')
exit $LASTEXITCODE
