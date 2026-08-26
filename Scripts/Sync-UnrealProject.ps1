param(
    [string]$ProjectPath = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")).Path "UTween.uproject"),
    [string]$EngineRoot,
    [switch]$BuildEditor,
    [switch]$CloseEditor
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-EngineRoot {
    param(
        [string]$ResolvedProjectPath,
        [string]$ExplicitEngineRoot
    )

    if ($ExplicitEngineRoot) {
        return (Resolve-Path $ExplicitEngineRoot).Path
    }

    $projectJson = Get-Content -Raw $ResolvedProjectPath | ConvertFrom-Json
    $engineAssociation = $projectJson.EngineAssociation

    if ($engineAssociation) {
        $candidate = Join-Path "C:\Program Files\Epic Games" ("UE_{0}" -f $engineAssociation)
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    $installed = Get-ChildItem "C:\Program Files\Epic Games" -Directory -Filter "UE_*" -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending

    if ($installed -and $installed.Count -gt 0) {
        return $installed[0].FullName
    }

    throw "Could not resolve an Unreal Engine installation. Pass -EngineRoot explicitly."
}

function Assert-Tool {
    param(
        [string]$PathValue,
        [string]$ToolName
    )

    if (-not (Test-Path $PathValue)) {
        throw "$ToolName not found: $PathValue"
    }
}

$resolvedProjectPath = (Resolve-Path $ProjectPath).Path
$resolvedEngineRoot = Resolve-EngineRoot -ResolvedProjectPath $resolvedProjectPath -ExplicitEngineRoot $EngineRoot

$buildBat = Join-Path $resolvedEngineRoot "Engine\Build\BatchFiles\Build.bat"

Assert-Tool -PathValue $buildBat -ToolName "Build.bat"

if ($CloseEditor) {
    $editorProcess = Get-Process UnrealEditor -ErrorAction SilentlyContinue
    if ($editorProcess) {
        Write-Host "Closing UnrealEditor to avoid Live Coding build lock..."
        $editorProcess | Stop-Process -Force
    }
}

Write-Host "Using Engine: $resolvedEngineRoot"
Write-Host "Project: $resolvedProjectPath"
Write-Host "Regenerating project files..."

& $buildBat -ProjectFiles -project="$resolvedProjectPath" -game -engine
if ($LASTEXITCODE -ne 0) {
    throw "Project file generation failed with exit code $LASTEXITCODE"
}

if ($BuildEditor) {
    $targetName = "{0}Editor" -f [System.IO.Path]::GetFileNameWithoutExtension($resolvedProjectPath)
    Write-Host "Building target: $targetName"
    & $buildBat $targetName Win64 Development "$resolvedProjectPath" -WaitMutex -NoHotReload
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
}

Write-Host "Sync complete."
