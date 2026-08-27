param(
    [string]$ProjectPath = (Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")).Path "UTween.uproject"),
    [string]$EngineRoot,
    [switch]$BuildEditor,
    [switch]$CloseEditor,
    [switch]$RunTests,
    [string]$TestFilter = "UTween"
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
        # Check direct folder match: e.g. "UE_5.8" or "UE_5.8.2"
        $candidate = Join-Path "C:\Program Files\Epic Games" ("UE_{0}" -f $engineAssociation)
        if (Test-Path $candidate) {
            return $candidate
        }

        # If association has patch version (e.g. 5.8.2), try Major.Minor prefix (UE_5.8)
        if ($engineAssociation -match '^(\d+\.\d+)') {
            $candidateMajorMinor = Join-Path "C:\Program Files\Epic Games" ("UE_{0}" -f $Matches[1])
            if (Test-Path $candidateMajorMinor) {
                return $candidateMajorMinor
            }
        }

        # Check LauncherInstalled.dat if it exists
        $launcherInstalledPath = "C:\ProgramData\Epic\UnrealEngineLauncher\LauncherInstalled.dat"
        if (Test-Path $launcherInstalledPath) {
            try {
                $launcherData = Get-Content -Raw $launcherInstalledPath | ConvertFrom-Json
                foreach ($item in $launcherData.InstallationList) {
                    if ($item.AppName -eq ("UE_{0}" -f $engineAssociation) -or 
                        $item.AppVersion -like ("{0}*" -f $engineAssociation)) {
                        if (Test-Path $item.InstallLocation) {
                            return $item.InstallLocation
                        }
                    }
                }
            } catch {
                # Fall through to directory search on error
            }
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

if ($RunTests) {
    $editorCmd = Join-Path $resolvedEngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    Assert-Tool -PathValue $editorCmd -ToolName "UnrealEditor-Cmd.exe"

    Write-Host "Running automation tests with filter: $TestFilter..."
    $execCmd = "Automation RunTests {0}; Quit" -f $TestFilter
    & $editorCmd "$resolvedProjectPath" -ExecCmds="$execCmd" -stdout -FullStdOutLogOutput -unattended -NullRHI -NoPause -buildmachine
    if ($LASTEXITCODE -ne 0) {
        throw "Automation tests failed with exit code $LASTEXITCODE"
    }
    Write-Host "Automation tests completed successfully."
}

Write-Host "Sync complete."
