param(
    [string]$Repository = (Resolve-Path "$PSScriptRoot/../..").Path,
    [string]$Output = 'out/gui-review-current',
    [ValidateSet('game','lobby')][string]$Suite = 'game'
)
$ErrorActionPreference = 'Stop'
$runtime = Join-Path $Repository 'out/mud3-01'
$destination = Join-Path $Repository $Output
New-Item -ItemType Directory -Path $destination -Force | Out-Null
$binary = Join-Path $runtime 'Mir2Ex-gallery-review.exe'
Copy-Item -LiteralPath (Join-Path $Repository 'build-ninja-sqlite/Mir2Ex.exe') -Destination $binary -Force
$modes = @(
    'hud-expanded','inventory-panel','status-panel','status-elements','skills-panel','quick-items','skill-key-menu','skill-description',
    'group-panel','group-create','group-invite','group-remove','guild-overview','guild-members','guild-notice','guild-denied','guild-notice-editor','guild-members-editor',
    'options-interaction','mount-panel','quest-empty','quest-list','trade-empty','trade-list','trade-confirmed','trade-gold',
    'npc-dialogue','npc-input','npc-password','shop-goods','shop-offers','shop-quantity','storage-list','storage-quantity','merchant-repair','merchant-sell',
    'mail-inbox','mail-read','mail-compose','mail-gift','consignment-empty','consignment-list','consignment-registration','consignment-confirm',
    'portrait-empty','portrait-preview','siege','siege-load','exit-program','exit-character','minimap-small','minimap-large'
)
if ($Suite -eq 'lobby') {
    $modes = @(
        'server-groups','server-group-hover','server-group-selected','server-group-row','server-group-scroll','server-group-exit','server-group-login',
        'login','login-hover','login-pressed','login-error','login-servers','login-billing',
        'selection-roster-first','selection-roster-waiting','selection-roster-second','selection-roster-long-name','selection-roster-empty',
        'role-male-warrior','role-male-mage','role-male-tao','role-female-warrior','role-female-mage','role-female-tao','role-selection-hover','role-selection-pressed',
        'character-name-exists','character-full','character-create-failed','character-name-invalid','character-query-failed','character-start-failed',
        'character-disconnected','character-delete-confirm','character-delete-missing','character-start-missing','character-name-long','character-empty'
    )
}
$manifest = [ordered]@{
    suite = $Suite
    revision = (git -C $Repository rev-parse HEAD)
    binaryHash = (Get-FileHash -LiteralPath $binary -Algorithm SHA256).Hash
    created = [DateTime]::UtcNow.ToString('o')
    captures = @()
}
foreach ($mode in $modes) {
    $file = Join-Path $runtime "$mode.bmp"
    $started = [DateTime]::UtcNow
    $process = Start-Process -FilePath $binary -ArgumentList "--preview=$mode" -WorkingDirectory $runtime -WindowStyle Hidden -PassThru
    try {
        do {
            Start-Sleep -Milliseconds 250
            $fresh = (Test-Path -LiteralPath $file) -and (Get-Item -LiteralPath $file).LastWriteTimeUtc -ge $started
            if ($process.HasExited -and !$fresh) { throw "$mode exited without a screenshot ($($process.ExitCode))" }
        } until ($fresh -or ([DateTime]::UtcNow - $started).TotalSeconds -gt 20)
        if (!$fresh) { throw "$mode did not produce a fresh screenshot" }
        # The BMP write must finish before stopping this preview process.
        Start-Sleep -Milliseconds 500
        Copy-Item -LiteralPath $file -Destination (Join-Path $destination "$mode.bmp") -Force
        $manifest.captures += [ordered]@{ mode=$mode; file="$mode.bmp"; captured=(Get-Item -LiteralPath $file).LastWriteTimeUtc.ToString('o') }
        $manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $destination 'manifest.json') -Encoding UTF8
        Write-Output "$($manifest.captures.Count)/$($modes.Count) $mode"
    } finally {
        if (!$process.HasExited) { Stop-Process -Id $process.Id }
    }
}
$reports = Join-Path $destination 'reports'
New-Item -ItemType Directory -Path $reports -Force | Out-Null
Get-ChildItem -LiteralPath $runtime -Filter '*check.txt' -File |
    Where-Object { $_.LastWriteTimeUtc -ge [DateTime]::Parse($manifest.created).ToUniversalTime() } |
    ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $reports -Force }
