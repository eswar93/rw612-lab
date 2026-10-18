<#
.SYNOPSIS
  Build (and optionally flash) a freestanding NXP MCUXpresso SDK project for the FRDM-RW612.

.EXAMPLE
  .\tools\nxp-build.ps1 -Project 02-temp-freertos -Clean
  .\tools\nxp-build.ps1 -Project 02-temp-freertos -Flash

.NOTES
  Needs ARMGCC_DIR set (folder above the toolchain's bin directory) and SEGGER J-Link on PATH.
  Paths can be overridden with the environment variables MCUX_SDK_DIR and WEST_VENV.
#>
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [string]$Board = "frdmrw612",
    [switch]$Clean,
    [switch]$Flash
)

$ErrorActionPreference = "Stop"

$SdkDir = if ($env:MCUX_SDK_DIR) { $env:MCUX_SDK_DIR } else { "C:\dev\mcuxpresso-sdk\mcuxsdk" }
$Venv   = if ($env:WEST_VENV)    { $env:WEST_VENV }    else { "C:\dev\zephyrproject\.venv" }

$RepoDir  = Split-Path -Parent $PSScriptRoot
$App      = Join-Path $RepoDir "projects\$Project"
$BuildDir = Join-Path $App "build"

if (-not (Test-Path $App)) { throw "Project folder not found: $App" }
if (-not (Test-Path $SdkDir)) { throw "SDK folder not found: $SdkDir (set MCUX_SDK_DIR)" }
if (-not $env:ARMGCC_DIR) { throw "ARMGCC_DIR is not set. See docs/setup-windows-nxp.md" }

# Same effect as activating the venv, without PowerShell's "untrusted publisher" prompt
$env:VIRTUAL_ENV = $Venv
$env:PATH = "$Venv\Scripts;$env:PATH"

$Conf = (Join-Path $App "prj.conf") -replace '\\', '/'

Push-Location $SdkDir
try {
    if ($Clean -and (Test-Path $BuildDir)) { Remove-Item -Recurse -Force $BuildDir }

    west build -b $Board $App -d $BuildDir "-DCONF_FILE=$Conf" --config=debug --toolchain=armgcc
    if ($LASTEXITCODE -ne 0) { throw "west build failed (exit code $LASTEXITCODE)" }

    if ($Flash) {
        west flash -d $BuildDir -r jlink
        if ($LASTEXITCODE -ne 0) { throw "west flash failed (exit code $LASTEXITCODE)" }
    }
}
finally {
    Pop-Location
}
