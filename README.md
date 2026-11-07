# UTween

Lightweight runtime tweening and game-feel animation framework for **Unreal Engine 5** (inspired by DOTween and Godot Tweens).

UTween provides a fluent C++ API, convenience macros, and Blueprint nodes to smoothly animate Actors, Components, UMG Widgets, Materials, and arbitrary data types with zero garbage collection overhead.

---

## Features

- **Fluent C++ & Opt-In Macros**: Method chaining API (`UTween::ActorLocation(...) -> SetEase(...) -> OnComplete(...)`) and concise macros (`UTWEEN(...)`, `UTWEEN_POSITION(...)`).
- **Full Blueprint Support**: First-class Blueprint nodes (`Tween Actor Location`, `Tween Widget Scale`, etc.) and latent sequence actions.
- **Juice & Game-Feel Essentials**: One-liner `Punch` and `Shake` animations for Actors, Scene Components, and Widgets.
- **Deterministic Sequencing**: Chain (`Append`), group (`Join`), delay (`AppendInterval`), loop (`SetLoops`), and trigger callbacks (`AppendCallback`) across complex timelines.
- **Comprehensive Easing**: 31 standard easing curves (Linear, Sine, Quad, Cubic, Quart, Quint, Expo, Circ, Back, Elastic, Bounce).
- **UMG Render & Layout Animation**: Render translation, scale, rotation, opacity, tint/color, and `CanvasPanelSlot` position and size.
- **Dynamic Materials**: Animate scalar and vector parameters on `UMaterialInstanceDynamic`.
- **Lifecycle & GC Safe**: Targets are tracked via `TWeakObjectPtr`. Destroyed actors or widgets safely self-terminate tweens without crashing or leaking memory.

---

## Installation

1. Copy or clone the `Plugins/UTween` directory into your project's `Plugins/` folder:
   ```text
   YourProject/
   └── Plugins/
       └── UTween/
   ```
2. Add `"UTween"` to your module's `Build.cs` dependencies:
   ```csharp
   PublicDependencyModuleNames.AddRange(new string[] {
       "Core",
       "CoreUObject",
       "Engine",
       "UTween"
   });
   ```
3. Include the main header in your C++ files:
   ```cpp
   #include "UTween.h"
   // Optional: #include "UTweenMacros.h" for shorthand macros
   ```

---

## Quickstart & Code Examples

### 1. Fluent C++ API

```cpp
#include "UTween.h"

// Move an Actor to a target position with bounce easing
UTween::ActorLocation(MyActor, TargetLocation, 0.75f)
    .SetEase(ETweenEase::OutBounce)
    .SetDelay(0.1f)
    .OnComplete([]() {
        UE_LOG(LogTemp, Log, TEXT("Actor reached target!"));
    });

// Scale punch a button widget on click (Juice)
UTween::PunchScale(MyButtonWidget, FVector2D(0.25f, 0.25f), 0.35f, 10, 1.0f);

// Camera / screen shake
UTween::ShakePosition(CameraActor, 25.0f, 0.5f, 12, 90.0f);

// Fade dynamic material emissive scalar
UTween::MaterialScalar(MyMID, "EmissiveIntensity", 5.0f, 0.2f)
    .SetEase(ETweenEase::OutQuad)
    .SetLoops(2, ETweenLoopMode::Yoyo);

// Generic value animation via lambda
UTween::Value(0.0f, 100.0f, 1.0f, [](float Value) {
    // Custom logic driven each frame
});
```

### 2. Timeline Sequences

```cpp
// Create an orchestrated sequence of chained and parallel steps
UTween::Sequence()
    .Append(UTween::ActorScale(MyActor, FVector(1.2f), 0.2f).SetEase(ETweenEase::OutQuad))
    .Join(UTween::MaterialScalar(MyMID, "Glow", 10.0f, 0.2f))
    .AppendInterval(0.1f)
    .Append(UTween::ActorScale(MyActor, FVector(1.0f), 0.3f).SetEase(ETweenEase::OutBounce))
    .AppendCallback([]() {
        UE_LOG(LogTemp, Log, TEXT("Sequence finished!"));
    })
    .Play();
```

### 3. Convenience Macros (Opt-In)

```cpp
#include "UTweenMacros.h"

// One-liner property tweens
UTWEEN_POSITION(MyActor, StartPos, EndPos, 0.5f).SetEase(ETweenEase::OutCubic);
UTWEEN_SCALE(MyWidget, FVector2D(1.0f), FVector2D(1.5f), 0.25f);
```

### 4. Blueprint Usage

All core operations are exposed to Blueprints with identical semantics:
- `Tween Actor Location` / `Rotation` / `Scale`
- `Tween Widget Position` / `Scale` / `Color` / `Opacity`
- `Punch Position` / `Punch Scale` / `Shake Position`
- `Create Tween Sequence` -> `Sequence Append` / `Sequence Join` / `Sequence Play`

---

## Included Demo: Game-Feel Combat Sandbox

The project includes a top-down combat demo (`Source/UTweenProject/GameFeel/`) showcasing how UTween transforms raw gameplay mechanics into satisfying, responsive "game-feel" juice.

<!-- Demo Preview GIF Placeholder -->
<!-- Add demo preview GIF here: ![GameFeel Combat Showcase](Docs/Images/gamefeel_showcase.gif) -->

### Combat Mechanics & Tweens Demonstrated:
- **Player Attack Sequence**: Anticipation wind-up, sword slash arc trail expansion, dynamic squash/stretch, and follow-through recovery.
- **Hit Feedback**: Emissive damage flash on dynamic materials, directional knockback, and camera shake.
- **Hit-Stop Freeze**: Micro time-dilation pause on impactful weapon hits.
- **UI & HUD**: Smooth health bar easing, punch animations on combo counters, and hurt vignette flashes.

Press **`T`** or **`Tab`** while playing to toggle polish on and off in real time to instantly feel the difference UTween makes.

<!-- A/B Comparison GIF Placeholder -->
<!-- Add A/B comparison GIF here: ![Polish Toggle Comparison](Docs/Images/polish_comparison.gif) -->

---

## Performance & Benchmarks

UTween is made for high throughput and zero allocations in steady state:
- **Easing Evaluations**: ~150-250 million ops/second.
- **Tick Throughput**: ~0.02 to 0.05 microseconds per tween per frame.
- **Tag Filtering**: $O(1)$ amortized lookup table (`TagToTweenMap`) for rapid tag killing/pausing across 10,000+ active instances.

Run the benchmark suite headlessly or inside Unreal Editor:
```bash
python Scripts/Run-Benchmark.py
```

---

## Testing & Automation

Run the included unit and regression test suite:
```powershell
powershell -ExecutionPolicy Bypass -File Scripts/Sync-UnrealProject.ps1 -BuildEditor -RunTests
```
Or in Unreal Editor: **Tools -> Test Automation -> UTween**.

---

## Compatibility

- **Unreal Engine**: 5.4, 5.5, 5.6, 5.7, 5.8+ (Developed and validated on UE 5.8.2)
- **Platforms**: Windows, Mac, Linux, Consoles, Mobile (Zero external third-party dependencies)

---

## License

This project is licensed under the [MIT License](LICENSE).
