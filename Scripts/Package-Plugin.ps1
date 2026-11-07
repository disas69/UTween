<#
.SYNOPSIS
    Packages the standalone UTween plugin (Option A: Source distribution) into a clean zip archive ready for GitHub Releases.

.DESCRIPTION
    Copies Plugins/UTween (excluding Binaries, Intermediate, and temporary IDE files) along with LICENSE and README.md
    into a top-level UTween/ directory inside the zip archive so users can extract it directly into YourProject/Plugins/.

.PARAMETER OutputDir
    Output directory for the generated zip file. Defaults to <RepoRoot>/Releases.
#>
[CmdletBinding()]
param(
    [string]$OutputDir = ""
)

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$PluginRoot = Join-Path $RepoRoot "Plugins\UTween"
$UPluginFile = Join-Path $PluginRoot "UTween.uplugin"

if (-not (Test-Path $UPluginFile)) {
    throw "UTween.uplugin not found at: $UPluginFile"
}

$UPluginJson = Get-Content $UPluginFile -Raw | ConvertFrom-Json
$Version = if ($UPluginJson.VersionName) { $UPluginJson.VersionName } else { "1.0.0" }

if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = Join-Path $RepoRoot "Releases"
}

New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$ZipFileName = "UTween-v$Version-Source.zip"
$ZipFilePath = Join-Path $OutputDir $ZipFileName

$TempStaging = Join-Path ([System.IO.Path]::GetTempPath()) ("UTween_Package_" + [System.Guid]::NewGuid().ToString("N"))
$StagedPluginDir = Join-Path $TempStaging "UTween"

try {
    New-Item -ItemType Directory -Force -Path $StagedPluginDir | Out-Null

    Copy-Item -Path $UPluginFile -Destination (Join-Path $StagedPluginDir "UTween.uplugin") -Force

    $SourceDir = Join-Path $PluginRoot "Source"
    if (Test-Path $SourceDir) {
        Copy-Item -Path $SourceDir -Destination (Join-Path $StagedPluginDir "Source") -Recurse -Force
    }

    $ResourcesDir = Join-Path $PluginRoot "Resources"
    if (Test-Path $ResourcesDir) {
        Copy-Item -Path $ResourcesDir -Destination (Join-Path $StagedPluginDir "Resources") -Recurse -Force
    }

    $LicenseFile = Join-Path $RepoRoot "LICENSE"
    if (Test-Path $LicenseFile) {
        Copy-Item -Path $LicenseFile -Destination (Join-Path $StagedPluginDir "LICENSE") -Force
    }

    $ReadmeFile = Join-Path $RepoRoot "README.md"
    if (Test-Path $ReadmeFile) {
        Copy-Item -Path $ReadmeFile -Destination (Join-Path $StagedPluginDir "README.md") -Force
    }

    if (Test-Path $ZipFilePath) {
        Remove-Item $ZipFilePath -Force
    }

    Compress-Archive -Path $StagedPluginDir -DestinationPath $ZipFilePath -CompressionLevel Optimal

    $ZipInfo = Get-Item $ZipFilePath
    $SizeKB = [math]::Round($ZipInfo.Length / 1KB, 1)
    Write-Host "Successfully packaged UTween v$Version source release:" -ForegroundColor Green
    Write-Host "  Path: $ZipFilePath"
    Write-Host "  Size: $SizeKB KB"
}
finally {
    if (Test-Path $TempStaging) {
        Remove-Item $TempStaging -Recurse -Force -ErrorAction SilentlyContinue
    }
}
