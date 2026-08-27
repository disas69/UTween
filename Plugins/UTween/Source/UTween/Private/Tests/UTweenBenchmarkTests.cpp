#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "TweenInstance.h"
#include "TweenSequence.h"
#include "TweenHandle.h"
#include "UTweenSubsystem.h"
#include "UTween.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/Image.h"
#include "HAL/PlatformTime.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

#if WITH_DEV_AUTOMATION_TESTS

#define UTWEEN_BENCHMARK_FLAGS \
    (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace UTweenBenchmarkHelpers
{
    struct FBenchmarkScopeTimer
    {
        uint64 StartCycles;
        double& OutMs;

        FBenchmarkScopeTimer(double& InOutMs)
            : StartCycles(FPlatformTime::Cycles64())
            , OutMs(InOutMs)
        {
        }

        ~FBenchmarkScopeTimer()
        {
            const uint64 ElapsedCycles = FPlatformTime::Cycles64() - StartCycles;
            OutMs = FPlatformTime::ToMilliseconds64(ElapsedCycles);
        }
    };

    struct FBenchmarkWorld
    {
        UWorld* World = nullptr;
        UGameInstance* GI = nullptr;
        UTweenSubsystem* Subsystem = nullptr;

        bool Initialize()
        {
            World = UWorld::CreateWorld(EWorldType::Game, false);
            if (!World)
            {
                return false;
            }
            GI = NewObject<UGameInstance>();
            World->SetGameInstance(GI);
            Subsystem = NewObject<UTweenSubsystem>(GI);
            return Subsystem != nullptr;
        }

        void Cleanup()
        {
            if (Subsystem)
            {
                Subsystem->KillAll();
            }
            if (World)
            {
                World->DestroyWorld(false);
            }
        }
    };
}

// ---------------------------------------------------------------------------
// 1. Easing Micro-benchmark
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenBenchmarkEasingTest,
    "UTween.Benchmark.Easing", UTWEEN_BENCHMARK_FLAGS)

bool FUTweenBenchmarkEasingTest::RunTest(const FString& Parameters)
{
    constexpr int32 Iterations = 1000000;
    const ETweenEase TestEases[] = {
        ETweenEase::Linear,
        ETweenEase::OutQuad,
        ETweenEase::InOutCubic,
        ETweenEase::OutExpo,
        ETweenEase::OutElastic,
        ETweenEase::OutBounce
    };

    UE_LOG(LogUTween, Display, TEXT("=== UTween Benchmark: Easing Evaluation (1,000,000 iterations each) ==="));

    for (ETweenEase Ease : TestEases)
    {
        volatile float Sink = 0.0f;
        const uint64 StartCycles = FPlatformTime::Cycles64();

        for (int32 i = 0; i < Iterations; ++i)
        {
            const float T = static_cast<float>(i) / static_cast<float>(Iterations);
            Sink = FTweenEasing::Evaluate(Ease, T);
        }

        const uint64 ElapsedCycles = FPlatformTime::Cycles64() - StartCycles;
        const double ElapsedMs = FPlatformTime::ToMilliseconds64(ElapsedCycles);
        const double NsPerOp = (ElapsedMs * 1000000.0) / Iterations;
        const double MopsPerSec = (Iterations / (ElapsedMs / 1000.0)) / 1000000.0;

        UE_LOG(LogUTween, Display,
            TEXT("Ease [%-12s]: Total %6.2f ms | %6.2f ns/op | %6.2f Mops/s"),
            *UEnum::GetValueAsString(Ease), ElapsedMs, NsPerOp, MopsPerSec);
    }

    // Round-robin across all 31 eases
    {
        volatile float Sink = 0.0f;
        const uint64 StartCycles = FPlatformTime::Cycles64();

        for (int32 i = 0; i < Iterations; ++i)
        {
            const float T = static_cast<float>(i) / static_cast<float>(Iterations);
            const ETweenEase Ease = static_cast<ETweenEase>(i % 31);
            Sink = FTweenEasing::Evaluate(Ease, T);
        }

        const uint64 ElapsedCycles = FPlatformTime::Cycles64() - StartCycles;
        const double ElapsedMs = FPlatformTime::ToMilliseconds64(ElapsedCycles);
        const double NsPerOp = (ElapsedMs * 1000000.0) / Iterations;
        const double MopsPerSec = (Iterations / (ElapsedMs / 1000.0)) / 1000000.0;

        UE_LOG(LogUTween, Display,
            TEXT("Ease [RoundRobin x31]: Total %6.2f ms | %6.2f ns/op | %6.2f Mops/s"),
            ElapsedMs, NsPerOp, MopsPerSec);
    }

    return true;
}

// ---------------------------------------------------------------------------
// 2. Tween Creation & Registration Benchmark
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenBenchmarkCreationTest,
    "UTween.Benchmark.TweenCreation", UTWEEN_BENCHMARK_FLAGS)

bool FUTweenBenchmarkCreationTest::RunTest(const FString& Parameters)
{
    UTweenBenchmarkHelpers::FBenchmarkWorld BenchWorld;
    if (!BenchWorld.Initialize())
    {
        return false;
    }

    const int32 Counts[] = { 1000, 5000, 10000, 25000, 50000 };

    UE_LOG(LogUTween, Display, TEXT("=== UTween Benchmark: Tween Creation & Registration ==="));

    for (int32 Count : Counts)
    {
        volatile float Sink = 0.0f;
        double ElapsedMs = 0.0;

        {
            UTweenBenchmarkHelpers::FBenchmarkScopeTimer Timer(ElapsedMs);
            for (int32 i = 0; i < Count; ++i)
            {
                FTweenHandle Handle = UTween::Value(
                    BenchWorld.GI, 0.0f, 100.0f, 5.0f,
                    [&Sink](float V)
                    {
                        Sink = V;
                    });
            }
        }

        const double UsPerTween = (ElapsedMs * 1000.0) / Count;
        const double TweensPerSec = Count / (ElapsedMs / 1000.0);

        UE_LOG(LogUTween, Display,
            TEXT("Creation [%6d tweens]: %6.2f ms | %6.2f us/tween | %8.0f tweens/s"),
            Count, ElapsedMs, UsPerTween, TweensPerSec);

        TestEqual(TEXT("Subsystem active count matches created count"),
            BenchWorld.Subsystem->GetActiveTweenCount(), Count);

        BenchWorld.Subsystem->KillAll();
    }

    BenchWorld.Cleanup();
    return true;
}

// ---------------------------------------------------------------------------
// 3. Steady-State Tick Throughput Stress Test (1k to 50k tweens)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenBenchmarkTickThroughputTest,
    "UTween.Benchmark.TickThroughput", UTWEEN_BENCHMARK_FLAGS)

bool FUTweenBenchmarkTickThroughputTest::RunTest(const FString& Parameters)
{
    UTweenBenchmarkHelpers::FBenchmarkWorld BenchWorld;
    if (!BenchWorld.Initialize())
    {
        return false;
    }

    const int32 Counts[] = { 1000, 5000, 10000, 25000, 50000 };
    constexpr int32 WarmupTicks = 5;
    constexpr int32 MeasuredTicks = 100;
    constexpr float DeltaTime = 1.0f / 60.0f;

    UE_LOG(LogUTween, Display,
        TEXT("=== UTween Benchmark: Steady-State Tick Throughput (%d frames @ 60 FPS) ==="),
        MeasuredTicks);

    for (int32 Count : Counts)
    {
        volatile float Sink = 0.0f;

        // Register active tweens with long duration so they tick continuously
        for (int32 i = 0; i < Count; ++i)
        {
            UTween::Value(
                BenchWorld.GI, 0.0f, 1000.0f, 60.0f,
                [&Sink](float V)
                {
                    Sink = V;
                }).SetEase(ETweenEase::OutQuad);
        }

        // Warm-up frames to ensure any first-tick setup (bStartFired) finishes
        for (int32 w = 0; w < WarmupTicks; ++w)
        {
            BenchWorld.Subsystem->Tick(DeltaTime);
        }

        TArray<double> FrameTimesMs;
        FrameTimesMs.Reserve(MeasuredTicks);
        double TotalMs = 0.0;
        double MinFrameMs = 1e9;
        double MaxFrameMs = 0.0;

        for (int32 f = 0; f < MeasuredTicks; ++f)
        {
            const uint64 StartCycles = FPlatformTime::Cycles64();
            BenchWorld.Subsystem->Tick(DeltaTime);
            const uint64 ElapsedCycles = FPlatformTime::Cycles64() - StartCycles;
            const double FrameMs = FPlatformTime::ToMilliseconds64(ElapsedCycles);

            FrameTimesMs.Add(FrameMs);
            TotalMs += FrameMs;
            MinFrameMs = FMath::Min(MinFrameMs, FrameMs);
            MaxFrameMs = FMath::Max(MaxFrameMs, FrameMs);
        }

        const double AvgFrameMs = TotalMs / MeasuredTicks;
        const double UsPerTweenPerFrame = (AvgFrameMs * 1000.0) / Count;
        const double EffectiveThroughput = (static_cast<double>(Count) * MeasuredTicks) / (TotalMs / 1000.0);

        UE_LOG(LogUTween, Display,
            TEXT("Tick [%6d tweens]: Avg %6.2f ms/frame (Min: %6.2f, Max: %6.2f) | %6.3f us/tween/frame | %9.0f evals/s"),
            Count, AvgFrameMs, MinFrameMs, MaxFrameMs, UsPerTweenPerFrame, EffectiveThroughput);

        BenchWorld.Subsystem->KillAll();
    }

    BenchWorld.Cleanup();
    return true;
}

// ---------------------------------------------------------------------------
// 4. Actor & Widget Target Overhead Benchmark (TWeakObjectPtr validation)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenBenchmarkActorAndWidgetTest,
    "UTween.Benchmark.ActorAndWidget", UTWEEN_BENCHMARK_FLAGS)

bool FUTweenBenchmarkActorAndWidgetTest::RunTest(const FString& Parameters)
{
    UTweenBenchmarkHelpers::FBenchmarkWorld BenchWorld;
    if (!BenchWorld.Initialize())
    {
        return false;
    }

    constexpr int32 TargetCount = 2000;
    constexpr int32 MeasuredTicks = 60;
    constexpr float DeltaTime = 1.0f / 60.0f;

    UE_LOG(LogUTween, Display, TEXT("=== UTween Benchmark: Target Validation & Transform Overhead (%d targets) ==="), TargetCount);

    // 1. Pure Value baseline
    {
        volatile float Sink = 0.0f;
        for (int32 i = 0; i < TargetCount; ++i)
        {
            UTween::Value(
                BenchWorld.GI, 0.0f, 100.0f, 10.0f,
                [&Sink](float V)
                {
                    Sink = V;
                });
        }
        BenchWorld.Subsystem->Tick(DeltaTime); // Warmup

        double TotalMs = 0.0;
        for (int32 f = 0; f < MeasuredTicks; ++f)
        {
            const uint64 StartCycles = FPlatformTime::Cycles64();
            BenchWorld.Subsystem->Tick(DeltaTime);
            TotalMs += FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
        }
        const double AvgMs = TotalMs / MeasuredTicks;
        UE_LOG(LogUTween, Display, TEXT("Baseline (Value<float>):        Avg %6.2f ms/frame | %6.3f us/tween/frame"),
            AvgMs, (AvgMs * 1000.0) / TargetCount);

        BenchWorld.Subsystem->KillAll();
    }

    // 2. Real AActor Location Tweens
    {
        TArray<AActor*> Actors;
        Actors.Reserve(TargetCount);
        for (int32 i = 0; i < TargetCount; ++i)
        {
            AActor* Actor = BenchWorld.World->SpawnActor<AActor>();
            if (Actor)
            {
                USceneComponent* RootComp = NewObject<USceneComponent>(Actor);
                Actor->SetRootComponent(RootComp);
                RootComp->RegisterComponent();
                Actors.Add(Actor);

                UTween::ActorLocation(Actor, FVector(100.0f, 200.0f, 300.0f), 10.0f);
            }
        }
        BenchWorld.Subsystem->Tick(DeltaTime); // Warmup

        double TotalMs = 0.0;
        for (int32 f = 0; f < MeasuredTicks; ++f)
        {
            const uint64 StartCycles = FPlatformTime::Cycles64();
            BenchWorld.Subsystem->Tick(DeltaTime);
            TotalMs += FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
        }
        const double AvgMs = TotalMs / MeasuredTicks;
        UE_LOG(LogUTween, Display, TEXT("ActorLocation (TWeakObjectPtr): Avg %6.2f ms/frame | %6.3f us/tween/frame"),
            AvgMs, (AvgMs * 1000.0) / TargetCount);

        BenchWorld.Subsystem->KillAll();
    }

    // 3. Real UWidget Render Translation Tweens
    {
        TArray<UImage*> Widgets;
        Widgets.Reserve(TargetCount);
        for (int32 i = 0; i < TargetCount; ++i)
        {
            UImage* Widget = NewObject<UImage>(BenchWorld.GI);
            if (Widget)
            {
                Widgets.Add(Widget);
                UTween::WidgetPosition(Widget, FVector2D(100.0f, 100.0f), 10.0f);
            }
        }
        BenchWorld.Subsystem->Tick(DeltaTime); // Warmup

        double TotalMs = 0.0;
        for (int32 f = 0; f < MeasuredTicks; ++f)
        {
            const uint64 StartCycles = FPlatformTime::Cycles64();
            BenchWorld.Subsystem->Tick(DeltaTime);
            TotalMs += FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
        }
        const double AvgMs = TotalMs / MeasuredTicks;
        UE_LOG(LogUTween, Display, TEXT("WidgetPosition (RenderTransform): Avg %6.2f ms/frame | %6.3f us/tween/frame"),
            AvgMs, (AvgMs * 1000.0) / TargetCount);

        BenchWorld.Subsystem->KillAll();
    }

    BenchWorld.Cleanup();
    return true;
}

// ---------------------------------------------------------------------------
// 5. Sequence Scale Benchmark (Timeline & Action Slicing)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenBenchmarkSequenceScaleTest,
    "UTween.Benchmark.SequenceScale", UTWEEN_BENCHMARK_FLAGS)

bool FUTweenBenchmarkSequenceScaleTest::RunTest(const FString& Parameters)
{
    UTweenBenchmarkHelpers::FBenchmarkWorld BenchWorld;
    if (!BenchWorld.Initialize())
    {
        return false;
    }

    constexpr int32 SequenceCount = 1000;
    constexpr int32 MeasuredTicks = 60;
    constexpr float DeltaTime = 1.0f / 60.0f;

    UE_LOG(LogUTween, Display, TEXT("=== UTween Benchmark: Sequence Scale (%d sequences with 4 steps) ==="), SequenceCount);

    volatile float Sink = 0.0f;

    for (int32 i = 0; i < SequenceCount; ++i)
    {
        FTweenSequenceHandle Seq = UTween::Sequence(BenchWorld.GI);
        Seq.Append(UTween::Value(BenchWorld.GI, 0.0f, 100.0f, 0.5f, [&Sink](float V) { Sink = V; }))
           .Join(UTween::Value(BenchWorld.GI, 100.0f, 200.0f, 0.5f, [&Sink](float V) { Sink = V; }))
           .AppendInterval(0.1f)
           .Append(UTween::Value(BenchWorld.GI, 200.0f, 300.0f, 0.5f, [&Sink](float V) { Sink = V; }));
    }

    BenchWorld.Subsystem->Tick(DeltaTime); // Warmup

    double TotalMs = 0.0;
    double MinFrameMs = 1e9;
    double MaxFrameMs = 0.0;

    for (int32 f = 0; f < MeasuredTicks; ++f)
    {
        const uint64 StartCycles = FPlatformTime::Cycles64();
        BenchWorld.Subsystem->Tick(DeltaTime);
        const double FrameMs = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);

        TotalMs += FrameMs;
        MinFrameMs = FMath::Min(MinFrameMs, FrameMs);
        MaxFrameMs = FMath::Max(MaxFrameMs, FrameMs);
    }

    const double AvgFrameMs = TotalMs / MeasuredTicks;
    const double UsPerSeqPerFrame = (AvgFrameMs * 1000.0) / SequenceCount;

    UE_LOG(LogUTween, Display,
        TEXT("Sequences [%6d active]: Avg %6.2f ms/frame (Min: %6.2f, Max: %6.2f) | %6.3f us/sequence/frame"),
        SequenceCount, AvgFrameMs, MinFrameMs, MaxFrameMs, UsPerSeqPerFrame);

    BenchWorld.Cleanup();
    return true;
}

// ---------------------------------------------------------------------------
// 6. Churn & Cleanup Stress Test (Continuous spawn + completion + RemoveAll)
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenBenchmarkChurnStressTest,
    "UTween.Benchmark.ChurnAndCleanup", UTWEEN_BENCHMARK_FLAGS)

bool FUTweenBenchmarkChurnStressTest::RunTest(const FString& Parameters)
{
    UTweenBenchmarkHelpers::FBenchmarkWorld BenchWorld;
    if (!BenchWorld.Initialize())
    {
        return false;
    }

    constexpr int32 SpawnsPerFrame = 500;
    constexpr int32 TotalFrames = 100;
    constexpr float TweenDuration = 2.0f / 60.0f; // 2 frames duration -> high completion churn
    constexpr float DeltaTime = 1.0f / 60.0f;

    UE_LOG(LogUTween, Display,
        TEXT("=== UTween Benchmark: Churn & Cleanup Stress (%d spawns/frame, %d frames, %d total tweens) ==="),
        SpawnsPerFrame, TotalFrames, SpawnsPerFrame * TotalFrames);

    volatile float Sink = 0.0f;
    double TotalMs = 0.0;
    double MinFrameMs = 1e9;
    double MaxFrameMs = 0.0;

    for (int32 f = 0; f < TotalFrames; ++f)
    {
        const uint64 StartCycles = FPlatformTime::Cycles64();

        for (int32 s = 0; s < SpawnsPerFrame; ++s)
        {
            UTween::Value(
                BenchWorld.GI, 0.0f, 1.0f, TweenDuration,
                [&Sink](float V)
                {
                    Sink = V;
                });
        }

        BenchWorld.Subsystem->Tick(DeltaTime);

        const double FrameMs = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64() - StartCycles);
        TotalMs += FrameMs;
        MinFrameMs = FMath::Min(MinFrameMs, FrameMs);
        MaxFrameMs = FMath::Max(MaxFrameMs, FrameMs);
    }

    const double AvgFrameMs = TotalMs / TotalFrames;

    UE_LOG(LogUTween, Display,
        TEXT("Churn Stress [%d frames]: Avg %6.2f ms/frame (Min: %6.2f, Max: %6.2f) | Total: %6.2f ms"),
        TotalFrames, AvgFrameMs, MinFrameMs, MaxFrameMs, TotalMs);

    BenchWorld.Cleanup();
    return true;
}

// ---------------------------------------------------------------------------
// 7. Subsystem Tag Filtering Benchmark
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenBenchmarkTagFilteringTest,
    "UTween.Benchmark.TagFiltering", UTWEEN_BENCHMARK_FLAGS)

bool FUTweenBenchmarkTagFilteringTest::RunTest(const FString& Parameters)
{
    UTweenBenchmarkHelpers::FBenchmarkWorld BenchWorld;
    if (!BenchWorld.Initialize())
    {
        return false;
    }

    constexpr int32 TweenCount = 10000;
    const FTweenTag Tags[] = {
        FTweenTag(FName(TEXT("UI"))),
        FTweenTag(FName(TEXT("Player"))),
        FTweenTag(FName(TEXT("FX"))),
        FTweenTag(FName(TEXT("Enemy"))),
        FTweenTag(FName(TEXT("Camera")))
    };
    constexpr int32 NumTags = UE_ARRAY_COUNT(Tags);

    volatile float Sink = 0.0f;
    for (int32 i = 0; i < TweenCount; ++i)
    {
        FTweenHandle H = UTween::Value(BenchWorld.GI, 0.0f, 100.0f, 10.0f, [&Sink](float V) { Sink = V; });
        H.SetTag(Tags[i % NumTags]);
    }

    UE_LOG(LogUTween, Display, TEXT("=== UTween Benchmark: Tag Filtering (%d tweens, 5 tags) ==="), TweenCount);

    double PauseMs = 0.0;
    {
        UTweenBenchmarkHelpers::FBenchmarkScopeTimer Timer(PauseMs);
        BenchWorld.Subsystem->PauseByTag(FTweenTag(FName(TEXT("UI"))));
    }

    double ResumeMs = 0.0;
    {
        UTweenBenchmarkHelpers::FBenchmarkScopeTimer Timer(ResumeMs);
        BenchWorld.Subsystem->ResumeByTag(FTweenTag(FName(TEXT("UI"))));
    }

    double KillMs = 0.0;
    {
        UTweenBenchmarkHelpers::FBenchmarkScopeTimer Timer(KillMs);
        BenchWorld.Subsystem->KillByTag(FTweenTag(FName(TEXT("UI"))));
    }

    UE_LOG(LogUTween, Display,
        TEXT("Tag Filtering: PauseByTag %6.3f ms | ResumeByTag %6.3f ms | KillByTag %6.3f ms"),
        PauseMs, ResumeMs, KillMs);

    BenchWorld.Cleanup();
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
