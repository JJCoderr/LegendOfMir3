# 技能专项统一入口：默认全量；-Skill 仅运行所选编号的图形用例。
[CmdletBinding()]
param(
    [int]$Skill = 0,
    [switch]$Screenshots,
    [string]$Godot = 'Godot_v4.7.2-stable_win64_console.exe'
)
$ErrorActionPreference = 'Stop'
$clientRoot = Split-Path -Parent $PSScriptRoot
$godotExecutable = (Get-Command $Godot -ErrorAction Stop).Source
$checks = Join-Path $clientRoot 'build/Release/hud_checks.exe'
if ($Skill -eq 0) {
    foreach ($subset in @('--spell', '--skill-wire', '--action-reply')) {
        & $checks $subset
        if ($LASTEXITCODE -ne 0) { throw "技能模型验收失败：$subset" }
    }
}
$scripts = if ($Skill -eq 0) { @('skill_wire.gd', 'skill_casting.gd', 'forced_motion.gd') } else { @('skill_casting.gd') }
foreach ($script in $scripts) {
    $arguments = @('--path', (Join-Path $clientRoot 'godot'), '--script', (Join-Path $PSScriptRoot $script), '--')
    if ($Skill -ne 0) { $arguments += "--skill=$Skill" }
    if ($Screenshots) { $arguments += '--screenshots' }
    & $godotExecutable @arguments
    if ($LASTEXITCODE -ne 0) { throw "技能图形验收失败：$script，筛选=$Skill" }
}
Write-Output $(if ($Skill -eq 0) { 'SKILL_SUITE_ALL_OK' } else { "SKILL_SUITE_SINGLE_OK skill=$Skill" })
