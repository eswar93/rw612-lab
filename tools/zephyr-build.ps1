<#
.SYNOPSIS
  Build (and optionally flash) a Zephyr project from this repo for the FRDM-RW612.

.EXAMPLE
  .\tools\zephyr-build.ps1 -Project 01-blinky-zephyr
  .\tools\zephyr-build.ps1 -Project 03-ml-mqtt-telemetry -Clean -Flash

.NOTES
  Paths can be overridden with the environment variables ZEPHYR_WORKSPACE and WEST_VENV.
#>
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Board = "frdm_rw612",
    [switch]$Clean,
    [switch]$Flash
)

$ErrorActionPreference = "Stop"

$Workspace = if ($env:ZEPHYR_WORKSPACE) { $env:ZEPHYR_WORKSPACE } else { "C:\dev\zephyrproject" }
$Venv      = if ($env:WEST_VENV)        { $env:WEST_VENV }        else { Join-Path $Workspace ".venv" }

$RepoDir  = Split-Path -Parent $PSScriptRoot
$App      = Join-Path $RepoDir "projects\$Project"
$BuildDir = Join-Path $App "build"

if (-not (Test-Path $App)) { throw "Project folder not found: $App" }
if (-not (Test-Path (Join-Path $Workspace "zephyr"))) { throw "Zephyr workspace not found: $Workspace (set ZEPHYR_WORKSPACE)" }

# Same effect as activating the venv, without PowerShell's "untrusted publisher" prompt
$env:VIRTUAL_ENV = $Venv
$env:PATH = "$Venv\Scripts;$env:PATH"

Push-Location (Join-Path $Workspace "zephyr")
try {
    if ($Clean -and (Test-Path $BuildDir)) { Remove-Item -Recurse -Force $BuildDir }

    # Local secrets (Wi-Fi password, broker address) live in a git-ignored secrets.conf next to the project
    $Secrets = Join-Path $App "secrets.conf"
    $Extra = @()
    if (Test-Path $Secrets) {
        Write-Host "Using local secrets.conf (the firmware will contain the Wi-Fi password: do not share it)"
        $Extra = @("--", ("-DEXTRA_CONF_FILE=" + ($Secrets -replace '\\', '/')))
    }

    west build -b $Board $App -d $BuildDir -p auto @Extra
    if ($LASTEXITCODE -ne 0) { throw "west build failed (exit code $LASTEXITCODE)" }

    if ($Flash) {
        west flash -d $BuildDir
        if ($LASTEXITCODE -ne 0) { throw "west flash failed (exit code $LASTEXITCODE)" }
    }
}
finally {
    Pop-Location
}
