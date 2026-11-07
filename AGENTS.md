# AGENTS.md — Developer & Agent Reference Guide for UTween

This document is the primary reference for AI agents and human developers working in the `UTween` repository.

---

## 1. Project Overview

`UTween` is a high-performance runtime tween and game-feel feedback animation framework for Unreal Engine 5.8 (5.8.2) (inspired by Unity's DOTween and Godot Tweens).

It provides a fluent C++ API, convenience macros, and Blueprint function libraries to animate:
- **UMG Widgets**: Render translation, scale, rotation, opacity, color/tint, and Canvas Slot layout (position/size).
- **Actors**: World location, rotation (quaternion shortest-path slerp with multi-turn Euler fallback), and scale.
- **Scene Components**: Relative location, relative rotation, and relative scale.
- **Materials**: Scalar and vector parameters on `UMaterialInstanceDynamic`.
- **Generic Values**: Arbitrary `float`, `FVector2D`, `FVector`, `FRotator`, and `FLinearColor` values via lambdas or member function pointers.
- **Juice & Game-Feel**: Dedicated Punch and Shake animations for widgets, actors, and components.
- **Sequences**: Chained and parallel timeline sequences with intervals, nested loops, and callbacks.

---

## 2. Directory Structure & Key Files

```text
UTween/
├── .agents/
│   └── mcp_config.json                         <-- Canonical Unreal Engine MCP configuration
├── Content/
│   ├── TweenDemo/                              <-- Standalone TweenDemo showcase assets (C++ & Blueprint)
│   │   ├── Maps/
│   │   │   ├── TweenDemo.umap                  <-- 10 placed studio cubes driven by C++ (AUTweenDemoManager)
│   │   │   └── TweenDemoBlueprint.umap         <-- 10 placed studio cubes driven by Blueprint (BP_TweenDemo_Blueprint)
│   │   ├── Blueprints/
│   │   │   ├── BP_TweenDemo.uasset             <-- C++ showcase manager actor
│   │   │   └── BP_TweenDemo_Blueprint.uasset   <-- Blueprint showcase manager actor (Demo0_Bounce .. Demo9_NestedTween)
│   │   ├── Meshes/
│   │   │   └── SM_TweenDemo_Cube.uasset        <-- Smooth-beveled studio cube mesh
│   │   └── Materials/
│   │       ├── M_TweenDemoBase.uasset          <-- Studio base material for TweenDemo
│   │       ├── M_TweenDemo_FloorGrid.uasset    <-- Checkerboard grid floor material
│   │       ├── MI_TweenDemo_FloorGrid.uasset   <-- Checkerboard grid floor instance
│   │       ├── MI_TweenDemo_Cube.uasset        <-- Default cube material instance
│   │       └── MI_TweenDemo_Cube_00..09.uasset <-- Per-cube vibrant material instances
│   └── GameFeel/                               <-- Standalone GameFeel combat sandbox assets
│       ├── Maps/
│       │   └── GameFeelMap.umap                <-- Top-down combat sandbox map
│       ├── Blueprints/                         <-- Player, Enemy, SlashArc, ImpactSparks, DamageNumber, GameMode BPs
│       ├── Input/                              <-- Enhanced Input actions and mapping context
│       ├── Meshes/                             <-- Capsule, Sword, BroadSword, and SlashArc meshes
│       └── Materials/                          <-- Studio character, weapon, VFX, dissolve, and floor materials
├── Plugins/
│   └── UTween/                                 <-- Core Plugin (Standalone & Redistributable)
│       ├── UTween.uplugin                      <-- Plugin descriptor (Runtime, EnabledByDefault)
│       └── Source/UTween/
│           ├── UTween.Build.cs                 <-- Module rules (Core, CoreUObject, Engine, UMG, Slate, SlateCore)
│           ├── Public/
│           │   ├── UTween.h                    <-- Primary static entry point (UTween::ActorLocation, UTween::Value, etc.)
│           │   ├── UTweenMacros.h              <-- Opt-in convenience macros (UTWEEN, UTWEEN_SEQUENCE, UTWEEN_PARALLEL, etc.)
│           │   ├── TweenHandle.h               <-- User-facing FTweenHandle control object (fluent setters, Kill/Pause/Resume)
│           │   ├── TweenSequence.h             <-- FTweenSequence runtime timeline and FTweenSequenceHandle
│           │   ├── TweenInstance.h             <-- Internal FTweenInstanceBase and TTweenInstance<T> runtime models
│           │   ├── TweenTypes.h                <-- ETweenEase, ETweenLoopMode, ETweenTimeMode, FTweenId, FTweenTag
│           │   ├── TweenEasing.h               <-- 31 easing curves (Quad, Cubic, Quart, Sine, Expo, Back, Bounce, Elastic)
│           │   ├── UTweenSubsystem.h           <-- UGameInstanceSubsystem managing active tweens, sequences, and ticking
│           │   ├── UTweenBlueprintLibrary.h    <-- Blueprint function library exposing nodes to Unreal Editor
│           │   ├── TweenBlueprintTypes.h       <-- FBPTweenHandle and FBPTweenSequenceHandle structs for Blueprint
│           │   ├── UTweenModule.h              <-- IModuleInterface definition and LogUTween declaration
│           │   └── UTweenSharedPCH.h           <-- Shared PCH header
│           └── Private/
│               ├── Tests/
│               │   ├── UTweenTests.cpp         <-- Automation tests (easing, loops, sequences, lifecycle, juice)
│               │   └── UTweenBenchmarkTests.cpp<-- Stress benchmarks (easing, throughput, churn, tag filtering)
│               ├── UTween.cpp
│               ├── UTweenSubsystem.cpp
│               ├── TweenHandle.cpp
│               ├── TweenSequence.cpp
│               ├── TweenInstance.cpp
│               ├── TweenEasing.cpp
│               ├── TweenTypes.cpp
│               ├── UTweenBlueprintLibrary.cpp
│               └── UTweenModule.cpp
├── Source/
│   ├── UTweenProject/                          <-- Host project game module (depends on UTween)
│   │   ├── UTweenProject.Build.cs
│   │   ├── UTweenProject.h
│   │   ├── UTweenProject.cpp
│   │   ├── Examples/                           <-- Pure C++ & macro API usage examples
│   │   │   ├── UTweenDemoManager.h/.cpp        <-- Showcase manager driving 10 placed level cubes & world-space labels
│   │   │   ├── UTweenFluentExample.h/.cpp      <-- Pure fluent C++ API examples
│   │   │   └── UTweenMacrosExample.h/.cpp      <-- Macro API examples
│   │   └── GameFeel/                           <-- Complete game-feel combat sandbox demo
│   │       ├── GameFeelPlayerCharacter.h/.cpp  <-- Player with juice attack sequences, squash/stretch, and cam shake
│   │       ├── GameFeelEnemyCharacter.h/.cpp   <-- Enemy with hit-stop, flash, knockback, animated life bars, and dissolve
│   │       ├── GameFeelHUDWidget.h/.cpp        <-- Responsive UI with diamond player hearts and hurt vignettes
│   │       ├── GameFeelSlashArc.h/.cpp         <-- Sliced sword slash trail actor with scale/dissolve tweening
│   │       ├── GameFeelImpactSparks.h/.cpp     <-- Directional spark burst FX with randomized outward tweens
│   │       ├── GameFeelDamageNumber.h/.cpp     <-- Floating world damage counter (-20, -50) with pop & fade
│   │       ├── GameFeelPlayerController.h/.cpp <-- Top-down input and mouse aiming
│   │       └── GameFeelGameMode.h/.cpp         <-- Spawning, round management, and live polish toggle (T / Tab)
│   ├── UTween.Target.cs                        <-- Target rules for standalone game
│   └── UTweenEditor.Target.cs                  <-- Target rules for editor
├── Scripts/
│   ├── Sync-UnrealProject.ps1                  <-- PowerShell automation to regenerate project files, build, and test
│   ├── Sync-UnrealProject.bat                  <-- Batch wrapper
│   └── Run-Benchmark.py                        <-- Automation benchmark runner (via Unreal MCP or headless)
└── UTween.uproject                             <-- Engine association 5.8, enables UTween plugin and UTweenProject module
```

---

## 3. Architecture & Specification Summary

1. **Central Runtime Owner (`UTweenSubsystem`)**:
   - Subclass of `UGameInstanceSubsystem`.
   - Ticks via `FTSTicker::GetCoreTicker()` so tweens can execute even during world pause or time dilation when configured.
   - Manages arrays of `ActiveTweens` and `ActiveSequences` with $O(1)$ amortized tag index lookups (`TagToTweenMap`).
   - Provides global controls: `PauseAll()`, `ResumeAll()`, `KillAll()`, and tag-based operations (`KillByTag()`, `PauseByTag()`, `ResumeByTag()`).
   - Diagnostic counters (`GetActiveTweenCount()`, `GetActiveSequenceCount()`).

2. **Handle vs. Instance Separation**:
   - **`FTweenInstanceBase` / `TTweenInstance<T>`**: Internal runtime objects owned exclusively by the subsystem or sequence via `TSharedPtr`.
   - **`FTweenHandle`**: Lightweight user-facing control struct holding a `TWeakPtr<FTweenInstanceBase>` and `FTweenId`. All handle operations (`Pause`, `Resume`, `Kill`, `SetEase`, etc.) safely no-op if the underlying instance has completed or been destroyed.

3. **Lifecycle & GC Safety**:
   - All `UObject` targets (actors, components, widgets, dynamic material instances) are held via `TWeakObjectPtr`.
   - On tick, if `BoundTarget.IsStale()` is true, the tween is automatically killed.
   - Lambda callbacks capture weak pointers, preventing dangling references and use-after-free bugs.

4. **Sequencing (`TweenSequence.h`)**:
   - Real runtime timeline managing `FSequenceAction` steps (Tween, Interval, Callback).
   - When a tween is added to a sequence (`Append` or `Join`), the subsystem detaches its standalone ownership (`DetachTween`), transferring execution and timeline control entirely to the `FTweenSequence`.
   - Supports `Restart` and `Yoyo` loops on sequences (yoyo mirrors action start times and reverses child steps).

5. **Timing Model**:
   - `ETweenTimeMode::GameTime` (default): Uses tick delta affected by world time dilation and game pause.
   - `ETweenTimeMode::RealTime`: Uses `FApp::GetDeltaTime()`, independent of world dilation or pause (ideal for UI/menus).

---

## 4. Implementation Evolution & Technical Decisions

- **`int64` for IDs**: Unreal reflection (`UPROPERTY`) does not support `uint64`. `FTweenId` uses `int64` to maintain full Blueprint accessibility.
- **RTTI-Free Type Safety**: Unreal modules compile with `-fno-rtti`. Type-safe downcasting in `TTweenInstance<T>` is implemented using the sentinel address pattern (`TTweenValueTypeTag<T>::Sentinel`).
- **Hybrid Rotator Interpolation**: `UTweenLerp::LerpRotator` uses quaternion spherical linear interpolation (`FQuat::Slerp`) for sub-360-degree shortest-path orientations, and falls back to Euler interpolation when any axis delta indicates an intentional 360-degree or multi-turn spin (`|delta| >= 360` or identical start/end quaternions) to prevent 360-degree rotations from collapsing into 0-degree stationary degenerate paths.
- **Tag Index Acceleration**: `UTweenSubsystem` maintains an internal tag-to-tween index map (`TagToTweenMap`) updated on registration and unregistration, ensuring tag queries (`KillByTag`, `PauseByTag`) execute in $O(K)$ where $K$ is the matching tag count rather than scanning all active tweens ($O(N)$).
- **Tick Snapshotting & Compaction**: The subsystem snapshots the active collection count at the start of `Tick()`. New tweens registered inside callbacks are deferred to the next tick to prevent iterator invalidation. Dead instances are pruned using fast swap-removal compaction.
- **Module Name Separation**: To prevent module name collision in UnrealBuildTool (since the project is named `UTween`), the host project module is named `UTweenProject`, and the plugin module is named `UTween`.
- **Deinitialization Sequence**: On subsystem shutdown, sequences are killed first so their `OnKill` callbacks fire while still holding their child tweens, followed by clearing standalone tweens.
- **Telemetry & Stats**: Integrated with Unreal Engine stats under `STATGROUP_UTween` (`stat UTween` in console reveals active count and tick cycle cost).

---

## 5. Agent Coding Guidelines

1. **No Useless Comments**:
   - Do **NOT** write comments that narrate obvious operations (e.g. `// Create handle`, `// Set ease to linear`).
   - Only comment non-obvious design rationale, edge-case workarounds, or constraints that code cannot self-describe.

2. **Follow `.clang-format`**:
   - Adhere strictly to the repository's `.clang-format` rules.

3. **Lifecycle Discipline**:
   - Always use `TWeakObjectPtr` for any `UObject`-derived target.
   - Never capture raw `UObject*` pointers by value or reference in long-lived lambdas.

4. **Preserve Compatibility**:
   - Maintain parity between the fluent C++ API, convenience macros, and Blueprint nodes.

5. **C++ vs. Blueprint Architectural Separation**:
   - **C++ Responsibility**: Core gameplay logic, state machines, math, tween dispatchers/orchestration, lifecycle & GC safety, and exposed Blueprint-configurable properties (`UPROPERTY`) / events (`UFUNCTION`).
   - **Blueprint & Native Asset Responsibility**: Visual hierarchies, mesh components, materials, particle effects (Niagara/Cascade), audio cues, and visual asset parameter configuration.
   - **No Procedural Visual Instantiation in Code**: Never hardcode procedural shapes, particles, or mesh assemblies in C++ when a native Blueprint or Material asset should represent the entity. Always author assets via Unreal MCP tools and expose `TSubclassOf<AActor>` / `TSubclassOf<UUserWidget>` / `UMaterialInterface*` for Blueprint assignment.

---

## 6. Mandatory Post-Change Build Verification Protocol

> [!IMPORTANT]
> **EVERY TIME A CODE CHANGE IS MADE**, the agent **MUST** run the project build script to validate compilation and refresh the IDE project files.
>
> Run:
> ```powershell
> powershell -ExecutionPolicy Bypass -File Scripts/Sync-UnrealProject.ps1 -BuildEditor
> ```
> Or in cmd/batch:
> ```cmd
> Scripts\Sync-UnrealProject.bat -BuildEditor
> ```
>
> To run automation tests after building:
> ```powershell
> powershell -ExecutionPolicy Bypass -File Scripts/Sync-UnrealProject.ps1 -BuildEditor -RunTests
> ```
>
> To execute the performance benchmark suite:
> ```powershell
> python Scripts/Run-Benchmark.py
> ```

### What this script does:
1. Automatically discovers the local Unreal Engine 5.8 installation (`C:\Program Files\Epic Games\UE_5.8`).
2. Regenerates Visual Studio project files (`Build.bat -ProjectFiles`).
3. Compiles the `UTweenEditor` target (Development Win64) using `UnrealBuildTool`.
4. Optionally executes the automation test suite headlessly via `UnrealEditor-Cmd.exe` (`-RunTests`).
5. Guarantees that compilation errors are discovered immediately and that the user's IDE is kept completely in sync.
