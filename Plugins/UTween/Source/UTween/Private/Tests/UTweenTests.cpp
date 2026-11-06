#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "TweenInstance.h"
#include "TweenSequence.h"
#include "TweenHandle.h"
#include "UTweenSubsystem.h"
#include "UTween.h"
#include "Engine/GameInstance.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "UTweenBlueprintLibrary.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UTweenTestHelpers
{
    static constexpr float FloatTolerance = 1e-4f;

    static TSharedPtr<TTweenInstance<float>> MakeFloatTween(
        float From, float To, float Duration,
        float& OutCaptured,
        ETweenEase Ease = ETweenEase::Linear)
    {
        TSharedPtr<TTweenInstance<float>> I = MakeShared<TTweenInstance<float>>();
        I->StartValue = From;
        I->EndValue = To;
        I->Duration = Duration;
        I->Ease = Ease;
        I->bUseFrom = true;
        I->bAutoKill = false;
        I->LerpFunc = [](const float& A, const float& B, float Alpha) { return FMath::Lerp(A, B, Alpha); };
        I->ApplyFunc = [&OutCaptured](const float& V) { OutCaptured = V; };
        I->State = ETweenState::Playing;
        return I;
    }

    static TSharedPtr<TTweenInstance<FVector>> MakeVectorTween(
        const FVector& From, const FVector& To, float Duration,
        FVector& OutCaptured)
    {
        TSharedPtr<TTweenInstance<FVector>> I = MakeShared<TTweenInstance<FVector>>();
        I->StartValue = From;
        I->EndValue = To;
        I->Duration = Duration;
        I->Ease = ETweenEase::Linear;
        I->bUseFrom = true;
        I->bAutoKill = false;
        I->LerpFunc = [](const FVector& A, const FVector& B, float Alpha) { return FMath::Lerp(A, B, Alpha); };
        I->ApplyFunc = [&OutCaptured](const FVector& V) { OutCaptured = V; };
        I->State = ETweenState::Playing;
        return I;
    }

    static TSharedPtr<TTweenInstance<FRotator>> MakeRotatorTween(
        const FRotator& From, const FRotator& To, float Duration,
        FRotator& OutCaptured)
    {
        TSharedPtr<TTweenInstance<FRotator>> I = MakeShared<TTweenInstance<FRotator>>();
        I->StartValue = From;
        I->EndValue = To;
        I->Duration = Duration;
        I->Ease = ETweenEase::Linear;
        I->bUseFrom = true;
        I->bAutoKill = false;
        I->LerpFunc = &UTweenLerp::LerpRotator;
        I->ApplyFunc = [&OutCaptured](const FRotator& V) { OutCaptured = V; };
        I->State = ETweenState::Playing;
        return I;
    }
}

using namespace UTweenTestHelpers;

#define UTWEEN_TEST_FLAGS \
    (EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


// ---------------------------------------------------------------------------
// Tween tests
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenLinearFloatTest,
    "UTween.Tween.LinearFloatProgresses", UTWEEN_TEST_FLAGS)

bool FUTweenLinearFloatTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);

    I->Tick(0.25f);
    TestEqual(TEXT("25% alpha"), Captured, 25.f, FloatTolerance);

    I->Tick(0.25f);
    TestEqual(TEXT("50% alpha"), Captured, 50.f, FloatTolerance);

    I->Tick(0.5f);
    TestEqual(TEXT("100% alpha"), Captured, 100.f, FloatTolerance);
    TestTrue(TEXT("Completed"), I->IsComplete());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenVectorInterpolationTest,
    "UTween.Tween.VectorInterpolation", UTWEEN_TEST_FLAGS)

bool FUTweenVectorInterpolationTest::RunTest(const FString& Parameters)
{
    FVector Captured = FVector::ZeroVector;
    auto I = MakeVectorTween(FVector::ZeroVector, FVector(100.f, 200.f, 300.f), 1.f, Captured);

    I->Tick(0.5f);
    TestTrue(TEXT("Midpoint"),
        Captured.Equals(FVector(50.f, 100.f, 150.f), FloatTolerance));

    I->Tick(0.5f);
    TestTrue(TEXT("End"),
        Captured.Equals(FVector(100.f, 200.f, 300.f), FloatTolerance));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenRotator360SpinTest,
    "UTween.Tween.Rotator360ContinuousSpin", UTWEEN_TEST_FLAGS)

bool FUTweenRotator360SpinTest::RunTest(const FString& Parameters)
{
    FRotator Captured = FRotator::ZeroRotator;
    auto I = MakeRotatorTween(FRotator::ZeroRotator, FRotator(0.f, 360.f, 0.f), 1.f, Captured);

    I->Tick(0.5f);
    TestTrue(TEXT("Midpoint 180 deg"), FMath::IsNearlyEqual(Captured.Yaw, 180.f, 1e-2f));

    I->Tick(0.5f);
    TestTrue(TEXT("Full 360 deg spin"), FMath::IsNearlyEqual(Captured.Yaw, 360.f, 1e-2f));
    TestTrue(TEXT("Completed"), I->IsComplete());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenRotatorYoyoLoopTest,
    "UTween.Tween.RotatorYoyoLoop", UTWEEN_TEST_FLAGS)

bool FUTweenRotatorYoyoLoopTest::RunTest(const FString& Parameters)
{
    FRotator Captured = FRotator::ZeroRotator;
    auto I = MakeRotatorTween(FRotator::ZeroRotator, FRotator(0.f, 360.f, 0.f), 1.f, Captured);
    I->LoopMode = ETweenLoopMode::Yoyo;
    I->LoopCount = 1;

    I->Tick(1.f);
    TestTrue(TEXT("End of cycle 1 (360 deg)"), FMath::IsNearlyEqual(Captured.Yaw, 360.f, 1e-2f));

    I->Tick(0.5f);
    TestTrue(TEXT("Mid of yoyo reverse cycle (180 deg)"), FMath::IsNearlyEqual(Captured.Yaw, 180.f, 1e-2f));

    I->Tick(0.5f);
    TestTrue(TEXT("End of yoyo reverse cycle (0 deg)"), FMath::IsNearlyEqual(Captured.Yaw, 0.f, 1e-2f));
    TestTrue(TEXT("Completed after yoyo"), I->IsComplete());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenRotator720MultiTurnTest,
    "UTween.Tween.Rotator720MultiTurn", UTWEEN_TEST_FLAGS)

bool FUTweenRotator720MultiTurnTest::RunTest(const FString& Parameters)
{
    FRotator Captured = FRotator::ZeroRotator;
    auto I = MakeRotatorTween(FRotator::ZeroRotator, FRotator(720.f, 0.f, 0.f), 1.f, Captured);

    I->Tick(0.5f);
    TestTrue(TEXT("Midpoint 360 deg pitch"), FMath::IsNearlyEqual(Captured.Pitch, 360.f, 1e-2f));

    I->Tick(0.5f);
    TestTrue(TEXT("Full 720 deg pitch"), FMath::IsNearlyEqual(Captured.Pitch, 720.f, 1e-2f));
    TestTrue(TEXT("Completed"), I->IsComplete());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenEaseOutQuadTest,
    "UTween.Tween.EaseOutQuadDiffersFromLinear", UTWEEN_TEST_FLAGS)

bool FUTweenEaseOutQuadTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured, ETweenEase::OutQuad);

    I->Tick(0.5f);
    // OutQuad at 0.5 = 1 - (0.5)^2 = 0.75
    TestEqual(TEXT("Value at 50% time"), Captured, 75.f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenDelayTest,
    "UTween.Tween.DelayDefersStart", UTWEEN_TEST_FLAGS)

bool FUTweenDelayTest::RunTest(const FString& Parameters)
{
    float Captured = -1.f;
    bool bStartFired = false;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);
    I->Delay = 0.5f;
    I->OnStartCallback = [&bStartFired] { bStartFired = true; };

    I->Tick(0.25f);
    TestFalse(TEXT("OnStart not yet fired during delay"), bStartFired);
    TestEqual(TEXT("Value unchanged during delay"), Captured, -1.f, FloatTolerance);

    I->Tick(0.25f);
    TestTrue(TEXT("OnStart fired after delay elapsed"), bStartFired);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenCallbacksTest,
    "UTween.Tween.CallbacksFireInOrder", UTWEEN_TEST_FLAGS)

bool FUTweenCallbacksTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    int32 StartCount = 0;
    int32 UpdateCount = 0;
    int32 CompleteCount = 0;
    int32 KillCount = 0;

    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);
    I->bAutoKill = true;
    I->OnStartCallback = [&StartCount] { ++StartCount; };
    I->OnUpdateCallback = [&UpdateCount] { ++UpdateCount; };
    I->OnCompleteCallback = [&CompleteCount] { ++CompleteCount; };
    I->OnKillCallback = [&KillCount] { ++KillCount; };

    I->Tick(0.25f);
    I->Tick(0.25f);
    I->Tick(0.5f);

    TestEqual(TEXT("OnStart fires once"), StartCount, 1);
    TestEqual(TEXT("OnUpdate fires per tick"), UpdateCount, 3);
    TestEqual(TEXT("OnComplete fires once"), CompleteCount, 1);
    TestEqual(TEXT("OnKill fires once (auto-kill)"), KillCount, 1);
    TestTrue(TEXT("State is Killed after auto-kill"), I->IsKilled());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenPauseResumeTest,
    "UTween.Tween.PauseAndResume", UTWEEN_TEST_FLAGS)

bool FUTweenPauseResumeTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);

    I->Tick(0.25f);
    TestEqual(TEXT("25% progress"), Captured, 25.f, FloatTolerance);

    I->Pause();
    I->Tick(0.25f);
    TestEqual(TEXT("No progress while paused"), Captured, 25.f, FloatTolerance);

    I->Resume();
    I->Tick(0.25f);
    TestEqual(TEXT("Resumed progress"), Captured, 50.f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenKillTest,
    "UTween.Tween.KillStopsEvaluation", UTWEEN_TEST_FLAGS)

bool FUTweenKillTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);
    I->Tick(0.25f);

    I->Kill();
    const float BeforeTick = Captured;
    I->Tick(0.5f);

    TestTrue(TEXT("Killed state"), I->IsKilled());
    TestEqual(TEXT("No evaluation after kill"), Captured, BeforeTick, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenLoopRestartTest,
    "UTween.Tween.LoopRestart", UTWEEN_TEST_FLAGS)

bool FUTweenLoopRestartTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    int32 CompleteCount = 0;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);
    I->LoopMode = ETweenLoopMode::Restart;
    I->LoopCount = 1;
    I->OnCompleteCallback = [&CompleteCount] { ++CompleteCount; };

    I->Tick(1.f);
    TestEqual(TEXT("End of first cycle"), Captured, 100.f, FloatTolerance);
    TestEqual(TEXT("No completion after first cycle"), CompleteCount, 0);

    I->Tick(0.5f);
    TestEqual(TEXT("Midway of second cycle restarts"), Captured, 50.f, FloatTolerance);

    I->Tick(0.5f);
    TestEqual(TEXT("OnComplete after final cycle"), CompleteCount, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenLoopYoyoTest,
    "UTween.Tween.LoopYoyo", UTWEEN_TEST_FLAGS)

bool FUTweenLoopYoyoTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);
    I->LoopMode = ETweenLoopMode::Yoyo;
    I->LoopCount = 1;

    I->Tick(1.f);
    TestEqual(TEXT("End of first cycle"), Captured, 100.f, FloatTolerance);

    I->Tick(0.5f);
    TestEqual(TEXT("Yoyo mid-return"), Captured, 50.f, FloatTolerance);

    I->Tick(0.5f);
    TestEqual(TEXT("Yoyo back to start"), Captured, 0.f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenRelativeTest,
    "UTween.Tween.RelativeOffsetsEnd", UTWEEN_TEST_FLAGS)

bool FUTweenRelativeTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(10.f, 5.f, 1.f, Captured);
    I->bIsRelative = true;

    I->Tick(1.f);
    // Relative: End becomes Start + End = 15
    TestEqual(TEXT("Lands on Start+End"), Captured, 15.f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenRotatorShortestPathTest,
    "UTween.Tween.RotatorShortestPath", UTWEEN_TEST_FLAGS)

bool FUTweenRotatorShortestPathTest::RunTest(const FString& Parameters)
{
    FRotator Captured = FRotator::ZeroRotator;
    auto I = MakeRotatorTween(
        FRotator(0.f, 350.f, 0.f),
        FRotator(0.f, 10.f, 0.f),
        1.f, Captured);

    I->Tick(0.5f);
    // Shortest path 350 -> 10 passes through 0/360, not -170.
    const float Yaw = FRotator::NormalizeAxis(Captured.Yaw);
    TestTrue(TEXT("Yaw midpoint near 0"), FMath::Abs(Yaw) < 5.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenAutoKillCompletesTest,
    "UTween.Tween.AutoKillTransitionsToKilled", UTWEEN_TEST_FLAGS)

bool FUTweenAutoKillCompletesTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);
    I->bAutoKill = true;

    I->Tick(1.f);
    TestTrue(TEXT("Auto-killed on complete"), I->IsKilled());
    TestEqual(TEXT("Final value applied"), Captured, 100.f, FloatTolerance);
    return true;
}


// ---------------------------------------------------------------------------
// Sequence tests
// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceAppendSequentialTest,
    "UTween.Sequence.AppendRunsSequentially", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceAppendSequentialTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    float B = 0.f;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);
    auto T2 = MakeFloatTween(0.f, 100.f, 1.f, B);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->AddTweenAction(T1, false);
    Seq->AddTweenAction(T2, false);
    Seq->State = ETweenState::Playing;

    Seq->Tick(0.5f);
    TestEqual(TEXT("T1 at 50%"), A, 50.f, FloatTolerance);
    TestEqual(TEXT("T2 not started"), B, 0.f, FloatTolerance);

    // Cursor reaches T1 end at 1.0s: T1 completes, T2 is at start position (0.0s elapsed in T2).
    Seq->Tick(0.5f);
    TestEqual(TEXT("T1 complete"), A, 100.f, FloatTolerance);
    TestEqual(TEXT("T2 started at 0%"), B, 0.f, FloatTolerance);

    // Cursor reaches 1.5s: T2 ticks 0.5s (50%).
    Seq->Tick(0.5f);
    TestEqual(TEXT("T2 at 50%"), B, 50.f, FloatTolerance);

    // Cursor reaches 2.0s: T2 completes (100%), sequence completes.
    Seq->Tick(0.5f);
    TestEqual(TEXT("T2 complete"), B, 100.f, FloatTolerance);
    TestTrue(TEXT("Sequence completes"), Seq->IsComplete());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceJoinParallelTest,
    "UTween.Sequence.JoinRunsInParallel", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceJoinParallelTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    float B = 0.f;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);
    auto T2 = MakeFloatTween(0.f, 200.f, 1.f, B);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->AddTweenAction(T1, false);
    Seq->AddTweenAction(T2, /*bJoin=*/true);
    Seq->State = ETweenState::Playing;

    Seq->Tick(0.5f);
    TestEqual(TEXT("T1 parallel progress"), A, 50.f, FloatTolerance);
    TestEqual(TEXT("T2 parallel progress"), B, 100.f, FloatTolerance);

    Seq->Tick(0.5f);
    TestEqual(TEXT("T1 complete"), A, 100.f, FloatTolerance);
    TestEqual(TEXT("T2 complete"), B, 200.f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceIntervalTest,
    "UTween.Sequence.AppendIntervalDelaysNext", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceIntervalTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->AddIntervalAction(0.5f);
    Seq->AddTweenAction(T1, false);
    Seq->State = ETweenState::Playing;

    Seq->Tick(0.25f);
    TestEqual(TEXT("Tween hasn't started during interval"), A, 0.f, FloatTolerance);

    Seq->Tick(0.2f);
    TestEqual(TEXT("Still mid-interval"), A, 0.f, FloatTolerance);

    // Cursor crosses interval end at 0.5s; of the 0.55s delta, 0.05s finishes interval and 0.50s ticks tween.
    Seq->Tick(0.55f);
    TestEqual(TEXT("Tween started after interval (ticked 0.50s)"), A, 50.f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceCallbackTest,
    "UTween.Sequence.AppendCallbackFiresAtPosition", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceCallbackTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    bool bCallbackFired = false;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->AddTweenAction(T1, false);
    Seq->AddCallbackAction([&bCallbackFired] { bCallbackFired = true; });
    Seq->State = ETweenState::Playing;

    Seq->Tick(0.5f);
    TestFalse(TEXT("Callback not fired yet"), bCallbackFired);

    Seq->Tick(0.5f);
    TestTrue(TEXT("Callback fired after tween finished"), bCallbackFired);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceCompleteTest,
    "UTween.Sequence.OnCompleteFiresOnce", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceCompleteTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    int32 CompleteCount = 0;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->AddTweenAction(T1, false);
    Seq->OnCompleteCallback = [&CompleteCount] { ++CompleteCount; };
    Seq->State = ETweenState::Playing;

    Seq->Tick(0.5f);
    Seq->Tick(0.5f);
    Seq->Tick(0.1f);

    TestEqual(TEXT("OnComplete fires exactly once"), CompleteCount, 1);
    TestTrue(TEXT("Sequence marked completed"), Seq->IsComplete());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceKillCascadesTest,
    "UTween.Sequence.KillCascadesToChildren", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceKillCascadesTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->AddTweenAction(T1, false);
    Seq->State = ETweenState::Playing;
    Seq->Tick(0.3f);

    Seq->Kill();
    TestTrue(TEXT("Sequence killed"), Seq->IsKilled());
    TestTrue(TEXT("Child tween killed"), T1->IsKilled());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceLoopRestartTest,
    "UTween.Sequence.LoopRestart", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceLoopRestartTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    int32 CompleteCount = 0;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->AddTweenAction(T1, false);
    Seq->LoopMode = ETweenLoopMode::Restart;
    Seq->LoopCount = 1;
    Seq->OnCompleteCallback = [&CompleteCount] { ++CompleteCount; };
    Seq->State = ETweenState::Playing;

    Seq->Tick(1.f);
    TestEqual(TEXT("End of first cycle"), A, 100.f, FloatTolerance);
    TestEqual(TEXT("No completion yet"), CompleteCount, 0);

    Seq->Tick(0.5f);
    TestEqual(TEXT("Second cycle restarted"), A, 50.f, FloatTolerance);

    Seq->Tick(0.5f);
    TestEqual(TEXT("Completion after last cycle"), CompleteCount, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequencePauseResumeTest,
    "UTween.Sequence.PauseAndResume", UTWEEN_TEST_FLAGS)

bool FUTweenSequencePauseResumeTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->AddTweenAction(T1, false);
    Seq->State = ETweenState::Playing;

    Seq->Tick(0.25f);
    TestEqual(TEXT("Initial progress"), A, 25.f, FloatTolerance);

    Seq->Pause();
    Seq->Tick(0.5f);
    TestEqual(TEXT("Paused, no progress"), A, 25.f, FloatTolerance);

    Seq->Resume();
    Seq->Tick(0.25f);
    TestEqual(TEXT("Resumed progress"), A, 50.f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenDelayOvershootTest,
    "UTween.Tween.DelayOvershootPreserved", UTWEEN_TEST_FLAGS)

bool FUTweenDelayOvershootTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);
    I->Delay = 0.5f;

    // Frame of 0.75s: 0.5s consumed by delay, remaining 0.25s consumed by tween (progress = 25%)
    I->Tick(0.75f);
    TestEqual(TEXT("Progress after delay overshoot"), Captured, 25.f, FloatTolerance);
    TestEqual(TEXT("Elapsed time equals leftover delta"), I->Elapsed, 0.25f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenLoopOvershootPreservedTest,
    "UTween.Tween.LoopOvershootPreserved", UTWEEN_TEST_FLAGS)

bool FUTweenLoopOvershootPreservedTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);
    I->LoopMode = ETweenLoopMode::Restart;
    I->LoopCount = 1;

    // Tick 1.1s: completes cycle 1 (duration 1.0) and carries 0.1s over into cycle 2
    I->Tick(1.1f);
    TestEqual(TEXT("Elapsed preserves 0.1s overshoot in second cycle"), I->Elapsed, 0.1f, FloatTolerance);

    // Next tick of 0.1s advances elapsed to 0.2s (20% progress)
    I->Tick(0.1f);
    TestEqual(TEXT("Elapsed is 0.2s"), I->Elapsed, 0.2f, FloatTolerance);
    TestEqual(TEXT("Value reflects 20% progress"), Captured, 20.f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenPlayRestartCompleteTest,
    "UTween.Tween.PlayRestartAndComplete", UTWEEN_TEST_FLAGS)

bool FUTweenPlayRestartCompleteTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);
    FTweenHandle Handle(I);

    Handle.Complete();
    TestTrue(TEXT("Complete transitions to completed"), I->IsComplete());
    TestEqual(TEXT("Complete applies 100%"), Captured, 100.f, FloatTolerance);

    Handle.Restart();
    TestTrue(TEXT("Restart transitions to playing"), Handle.IsPlaying());
    TestEqual(TEXT("Restart resets elapsed"), I->Elapsed, 0.f, FloatTolerance);

    I->Tick(0.5f);
    TestEqual(TEXT("Ticks normally after restart"), Captured, 50.f, FloatTolerance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenTargetDestructionTest,
    "UTween.Tween.TargetDestructionKillsTween", UTWEEN_TEST_FLAGS)

bool FUTweenTargetDestructionTest::RunTest(const FString& Parameters)
{
    float Captured = 0.f;
    auto I = MakeFloatTween(0.f, 100.f, 1.f, Captured);

    UGameInstance* Dummy = NewObject<UGameInstance>();
    I->BoundTarget = Dummy;
    I->bHasTarget = true;

    TestTrue(TEXT("Target valid initially"), I->IsTargetValid());
    I->Tick(0.25f);
    TestEqual(TEXT("25% evaluated"), Captured, 25.f, FloatTolerance);

    I->BoundTarget.Reset();
    TestFalse(TEXT("Target invalid when cleared"), I->IsTargetValid());

    I->Tick(0.25f);
    TestTrue(TEXT("Tween killed due to invalid target"), I->IsKilled());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceFrameSpikeSlicesTest,
    "UTween.Sequence.FrameSpikeSlicesAccurately", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceFrameSpikeSlicesTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    float B = 0.f;
    auto T1 = MakeFloatTween(0.f, 100.f, 0.5f, A);
    auto T2 = MakeFloatTween(0.f, 100.f, 0.5f, B);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->AddTweenAction(T1, false);
    Seq->AddTweenAction(T2, false);
    Seq->State = ETweenState::Playing;

    // A single frame spike of 0.75s across TotalDuration 1.0s:
    // T1 (0 to 0.5) should receive exactly 0.5s and complete.
    // T2 (0.5 to 1.0) should receive exactly 0.25s (Cursor 0.75 - StartTime 0.5).
    Seq->Tick(0.75f);
    TestEqual(TEXT("T1 receives exactly its slice (0.5s)"), A, 100.f, FloatTolerance);
    TestTrue(TEXT("T1 marked complete"), T1->IsComplete());
    TestEqual(TEXT("T2 receives remaining slice (0.25s / 50%)"), B, 50.f, FloatTolerance);
    TestFalse(TEXT("T2 not complete"), T2->IsComplete());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSubsystemSequenceControlsTest,
    "UTween.Subsystem.SequencesAffectedByControls", UTWEEN_TEST_FLAGS)

bool FUTweenSubsystemSequenceControlsTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Seq->Tag = FTweenTag(TEXT("TestTag"));
    Seq->Play();
    Subsystem->RegisterSequence(Seq);

    Subsystem->PauseByTag(FTweenTag(TEXT("TestTag")));
    TestEqual(TEXT("Sequence paused by tag"), Seq->State, ETweenState::Paused);

    Subsystem->ResumeByTag(FTweenTag(TEXT("TestTag")));
    TestEqual(TEXT("Sequence resumed by tag"), Seq->State, ETweenState::Playing);

    Subsystem->PauseAll();
    TestEqual(TEXT("Sequence paused by PauseAll"), Seq->State, ETweenState::Paused);

    Subsystem->ResumeAll();
    TestEqual(TEXT("Sequence resumed by ResumeAll"), Seq->State, ETweenState::Playing);

    Subsystem->KillAll();
    TestTrue(TEXT("Sequence killed by KillAll"), Seq->IsKilled());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSubsystemKillAllInCallbackTest,
    "UTween.Subsystem.KillAllInsideCallbackDoesNotCrash", UTWEEN_TEST_FLAGS)

bool FUTweenSubsystemKillAllInCallbackTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    float A = 0.f;
    float B = 0.f;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);
    auto T2 = MakeFloatTween(0.f, 100.f, 1.f, B);

    T1->OnUpdateCallback = [Subsystem]
    {
        Subsystem->KillAll();
    };

    Subsystem->RegisterTween(T1);
    Subsystem->RegisterTween(T2);

    Subsystem->Tick(0.1f);

    TestTrue(TEXT("T1 killed"), T1->IsKilled());
    TestTrue(TEXT("T2 killed"), T2->IsKilled());
    TestEqual(TEXT("No active tweens remain"), Subsystem->GetActiveTweenCount(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenMultiWorldSubsystemRoutingTest,
    "UTween.MultiWorld.TargetRoutesToCorrectSubsystem", UTWEEN_TEST_FLAGS)

bool FUTweenMultiWorldSubsystemRoutingTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI1 = NewObject<UGameInstance>();
    UGameInstance* GI2 = NewObject<UGameInstance>();

    UTweenSubsystem* Sub1 = NewObject<UTweenSubsystem>(GI1);
    UTweenSubsystem* Sub2 = NewObject<UTweenSubsystem>(GI2);

    TestNotNull(TEXT("Sub1 created"), Sub1);
    TestNotNull(TEXT("Sub2 created"), Sub2);

    TestEqual(TEXT("Resolves to Sub1"), UTween::GetSubsystem(GI1), Sub1);
    TestEqual(TEXT("Resolves to Sub2"), UTween::GetSubsystem(GI2), Sub2);
    TestTrue(TEXT("Subsystems are distinct"), Sub1 != Sub2);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenDirectAttachDetachesFromSubsystemTest,
    "UTween.Sequence.DirectAttachDetachesFromSubsystem", UTWEEN_TEST_FLAGS)

bool FUTweenDirectAttachDetachesFromSubsystemTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    float Val = 0.f;
    auto TweenInst = MakeFloatTween(0.f, 100.f, 1.f, Val);
    Subsystem->RegisterTween(TweenInst);
    TestEqual(TEXT("Active tween in subsystem before sequence attach"), Subsystem->GetActiveTweenCount(), 1);

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    FTweenSequenceHandle SeqHandle(Seq);
    FTweenHandle TweenHandle(TweenInst);

    SeqHandle.Append(TweenHandle);

    TestEqual(TEXT("Active tween count becomes 0 after sequence attach"), Subsystem->GetActiveTweenCount(), 0);
    TestEqual(TEXT("Action count in sequence is 1"), Seq->Actions.Num(), 1);
    TestTrue(TEXT("Attached action instance matches"), Seq->Actions[0].TweenInstance == TweenInst);
    TestTrue(TEXT("Owning subsystem cleared on instance"), !TweenInst->OwningSubsystem.IsValid());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenNestedSubSequenceTest,
    "UTween.Sequence.NestedSubSequenceExecution", UTWEEN_TEST_FLAGS)

bool FUTweenNestedSubSequenceTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    float B = 0.f;
    float C = 0.f;

    auto T1 = MakeFloatTween(0.f, 100.f, 0.5f, A);
    auto T2 = MakeFloatTween(0.f, 100.f, 0.5f, B);
    auto T3 = MakeFloatTween(0.f, 100.f, 0.5f, C);

    // Child sequence has T1 and T2 sequentially: total duration 1.0s
    TSharedPtr<FTweenSequence> ChildSeq = MakeShared<FTweenSequence>();
    FTweenSequenceHandle ChildHandle(ChildSeq);
    ChildHandle.Append(FTweenHandle(T1));
    ChildHandle.Append(FTweenHandle(T2));
    TestEqual(TEXT("Child sequence duration"), ChildSeq->TotalDuration, 1.0f, FloatTolerance);

    // Parent sequence has ChildSeq, then T3: total duration 1.5s
    TSharedPtr<FTweenSequence> ParentSeq = MakeShared<FTweenSequence>();
    FTweenSequenceHandle ParentHandle(ParentSeq);
    ParentHandle.Append(ChildHandle);
    ParentHandle.Append(FTweenHandle(T3));
    TestEqual(TEXT("Parent sequence duration"), ParentSeq->TotalDuration, 1.5f, FloatTolerance);

    ParentSeq->State = ETweenState::Playing;

    // Tick 0.5s: Child's T1 completes (100%), T2 at 0%, T3 at 0%
    ParentSeq->Tick(0.5f);
    TestEqual(TEXT("T1 completes at 0.5s"), A, 100.f, FloatTolerance);
    TestEqual(TEXT("T2 at 0% at 0.5s"), B, 0.f, FloatTolerance);
    TestEqual(TEXT("T3 at 0% at 0.5s"), C, 0.f, FloatTolerance);

    // Tick 0.5s: Child's T2 completes (100%), Child completes, T3 starts at 0%
    ParentSeq->Tick(0.5f);
    TestEqual(TEXT("T2 completes at 1.0s"), B, 100.f, FloatTolerance);
    TestTrue(TEXT("Child sequence completes"), ChildSeq->IsComplete());
    TestEqual(TEXT("T3 at 0% at 1.0s"), C, 0.f, FloatTolerance);

    // Tick 0.5s: T3 completes (100%), Parent completes
    ParentSeq->Tick(0.5f);
    TestEqual(TEXT("T3 completes at 1.5s"), C, 100.f, FloatTolerance);
    TestTrue(TEXT("Parent sequence completes"), ParentSeq->IsComplete());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenNestedSubSequencePauseTest,
    "UTween.Sequence.NestedSubSequencePauseCascade", UTWEEN_TEST_FLAGS)

bool FUTweenNestedSubSequencePauseTest::RunTest(const FString& Parameters)
{
    float A = 0.f;
    auto T1 = MakeFloatTween(0.f, 100.f, 1.f, A);

    TSharedPtr<FTweenSequence> ChildSeq = MakeShared<FTweenSequence>();
    FTweenSequenceHandle ChildHandle(ChildSeq);
    ChildHandle.Append(FTweenHandle(T1));

    TSharedPtr<FTweenSequence> ParentSeq = MakeShared<FTweenSequence>();
    FTweenSequenceHandle ParentHandle(ParentSeq);
    ParentHandle.Append(ChildHandle);

    ParentSeq->State = ETweenState::Playing;
    ParentSeq->Tick(0.25f);
    TestEqual(TEXT("T1 at 25%"), A, 25.f, FloatTolerance);

    ParentSeq->Pause();
    TestEqual(TEXT("Parent paused"), ParentSeq->State, ETweenState::Paused);
    TestEqual(TEXT("Child paused by cascade"), ChildSeq->State, ETweenState::Paused);

    ParentSeq->Tick(0.25f);
    TestEqual(TEXT("No progress while paused"), A, 25.f, FloatTolerance);

    ParentSeq->Resume();
    TestEqual(TEXT("Parent resumed"), ParentSeq->State, ETweenState::Playing);
    TestEqual(TEXT("Child resumed by cascade"), ChildSeq->State, ETweenState::Playing);

    ParentSeq->Tick(0.25f);
    TestEqual(TEXT("Progress after resume"), A, 50.f, FloatTolerance);

    ParentSeq->Kill();
    TestTrue(TEXT("Parent killed"), ParentSeq->IsKilled());
    TestTrue(TEXT("Child killed by cascade"), ChildSeq->IsKilled());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenEasingMathExactnessTest,
    "UTween.Easing.MathExactness", UTWEEN_TEST_FLAGS)

bool FUTweenEasingMathExactnessTest::RunTest(const FString& Parameters)
{
    // InQuad: T^2 -> 0.5^2 = 0.25
    TestEqual(TEXT("InQuad at 0.5"), FTweenEasing::Evaluate(ETweenEase::InQuad, 0.5f), 0.25f, FloatTolerance);

    // OutQuad: 1 - (1-T)^2 -> 1 - 0.5^2 = 0.75
    TestEqual(TEXT("OutQuad at 0.5"), FTweenEasing::Evaluate(ETweenEase::OutQuad, 0.5f), 0.75f, FloatTolerance);

    // InOutQuad: at 0.25 -> 2 * 0.25^2 = 0.125
    TestEqual(TEXT("InOutQuad at 0.25"), FTweenEasing::Evaluate(ETweenEase::InOutQuad, 0.25f), 0.125f, FloatTolerance);
    // InOutQuad: at 0.75 -> 1 - (-2*0.75 + 2)^2 / 2 = 1 - 0.5^2 / 2 = 1 - 0.125 = 0.875
    TestEqual(TEXT("InOutQuad at 0.75"), FTweenEasing::Evaluate(ETweenEase::InOutQuad, 0.75f), 0.875f, FloatTolerance);

    // InCubic: T^3 -> 0.5^3 = 0.125
    TestEqual(TEXT("InCubic at 0.5"), FTweenEasing::Evaluate(ETweenEase::InCubic, 0.5f), 0.125f, FloatTolerance);

    // OutCubic: 1 - (1-T)^3 -> 1 - 0.5^3 = 0.875
    TestEqual(TEXT("OutCubic at 0.5"), FTweenEasing::Evaluate(ETweenEase::OutCubic, 0.5f), 0.875f, FloatTolerance);

    // InQuart: T^4 -> 0.5^4 = 0.0625
    TestEqual(TEXT("InQuart at 0.5"), FTweenEasing::Evaluate(ETweenEase::InQuart, 0.5f), 0.0625f, FloatTolerance);

    // OutQuart: 1 - (1-T)^4 -> 1 - 0.5^4 = 0.9375
    TestEqual(TEXT("OutQuart at 0.5"), FTweenEasing::Evaluate(ETweenEase::OutQuart, 0.5f), 0.9375f, FloatTolerance);

    // InQuint: T^5 -> 0.5^5 = 0.03125
    TestEqual(TEXT("InQuint at 0.5"), FTweenEasing::Evaluate(ETweenEase::InQuint, 0.5f), 0.03125f, FloatTolerance);

    // OutQuint: 1 - (1-T)^5 -> 1 - 0.5^5 = 0.96875
    TestEqual(TEXT("OutQuint at 0.5"), FTweenEasing::Evaluate(ETweenEase::OutQuint, 0.5f), 0.96875f, FloatTolerance);

    // Boundaries
    TestEqual(TEXT("EaseOutBack at 0"), FTweenEasing::Evaluate(ETweenEase::OutBack, 0.0f), 0.0f, FloatTolerance);
    TestEqual(TEXT("EaseOutBack at 1"), FTweenEasing::Evaluate(ETweenEase::OutBack, 1.0f), 1.0f, FloatTolerance);
    TestEqual(TEXT("EaseInOutBack at 0"), FTweenEasing::Evaluate(ETweenEase::InOutBack, 0.0f), 0.0f, FloatTolerance);
    TestEqual(TEXT("EaseInOutBack at 1"), FTweenEasing::Evaluate(ETweenEase::InOutBack, 1.0f), 1.0f, FloatTolerance);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenValueTypeEnumTest,
    "UTween.Types.ValueTypeEnumMatching", UTWEEN_TEST_FLAGS)

bool FUTweenValueTypeEnumTest::RunTest(const FString& Parameters)
{
    TTweenInstance<float> FloatInst;
    TestEqual(TEXT("Float instance type"), FloatInst.ValueType, ETweenValueType::Float);
    TestTrue(TEXT("Float IsType<float>"), FloatInst.IsType<float>());
    TestFalse(TEXT("Float IsType<FVector>"), FloatInst.IsType<FVector>());

    TTweenInstance<FVector2D> Vec2DInst;
    TestEqual(TEXT("Vector2D instance type"), Vec2DInst.ValueType, ETweenValueType::Vector2D);
    TestTrue(TEXT("Vector2D IsType<FVector2D>"), Vec2DInst.IsType<FVector2D>());

    TTweenInstance<FVector> VecInst;
    TestEqual(TEXT("Vector instance type"), VecInst.ValueType, ETweenValueType::Vector);
    TestTrue(TEXT("Vector IsType<FVector>"), VecInst.IsType<FVector>());

    TTweenInstance<FRotator> RotInst;
    TestEqual(TEXT("Rotator instance type"), RotInst.ValueType, ETweenValueType::Rotator);
    TestTrue(TEXT("Rotator IsType<FRotator>"), RotInst.IsType<FRotator>());

    TTweenInstance<FLinearColor> ColorInst;
    TestEqual(TEXT("LinearColor instance type"), ColorInst.ValueType, ETweenValueType::LinearColor);
    TestTrue(TEXT("LinearColor IsType<FLinearColor>"), ColorInst.IsType<FLinearColor>());

    // Test Handle.From() using the new enum
    auto SharedInst = MakeShared<TTweenInstance<float>>();
    FTweenHandle Handle(SharedInst);
    Handle.From(42.f);
    TestTrue(TEXT("From updated bUseFrom"), SharedInst->bUseFrom);
    TestEqual(TEXT("From updated StartValue"), SharedInst->StartValue, 42.f, FloatTolerance);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenUMGWidgetColorTest,
    "UTween.UMG.WidgetColorSupportedTypes", UTWEEN_TEST_FLAGS)

bool FUTweenUMGWidgetColorTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    // Test UImage color tweening
    UImage* Image = NewObject<UImage>(GI);
    Image->SetColorAndOpacity(FLinearColor::Black);
    FTweenHandle ImageHandle = UTween::WidgetColor(Image, FLinearColor::White, 1.0f);
    TestTrue(TEXT("ImageHandle valid"), ImageHandle.IsValid());
    Subsystem->Tick(1.0f);
    TestEqual(TEXT("Image color interpolated to White"), Image->GetColorAndOpacity(), FLinearColor::White);

    // Test UBorder color tweening
    UBorder* Border = NewObject<UBorder>(GI);
    Border->SetBrushColor(FLinearColor::Black);
    FTweenHandle BorderHandle = UTween::WidgetColor(Border, FLinearColor::Red, 1.0f);
    TestTrue(TEXT("BorderHandle valid"), BorderHandle.IsValid());
    Subsystem->Tick(1.0f);
    TestEqual(TEXT("Border color interpolated to Red"), Border->GetBrushColor(), FLinearColor::Red);

    // Test UTextBlock color tweening
    UTextBlock* TextBlock = NewObject<UTextBlock>(GI);
    TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
    FTweenHandle TextHandle = UTween::WidgetColor(TextBlock, FLinearColor::Green, 1.0f);
    TestTrue(TEXT("TextHandle valid"), TextHandle.IsValid());
    Subsystem->Tick(1.0f);
    TestEqual(TEXT("TextBlock color interpolated to Green"), TextBlock->GetColorAndOpacity().GetSpecifiedColor(), FLinearColor::Green);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenUMGCanvasSlotTest,
    "UTween.UMG.CanvasSlotPositionAndSize", UTWEEN_TEST_FLAGS)

bool FUTweenUMGCanvasSlotTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    UCanvasPanelSlot* Slot = NewObject<UCanvasPanelSlot>(GI);
    Slot->SetPosition(FVector2D(0.f, 0.f));
    Slot->SetSize(FVector2D(100.f, 100.f));

    FTweenHandle PosHandle = UTween::CanvasPosition(Slot, FVector2D(200.f, 300.f), 1.0f).SetEase(ETweenEase::Linear);
    FTweenHandle SizeHandle = UTween::CanvasSize(Slot, FVector2D(500.f, 600.f), 1.0f).SetEase(ETweenEase::Linear);

    TestTrue(TEXT("PosHandle valid"), PosHandle.IsValid());
    TestTrue(TEXT("SizeHandle valid"), SizeHandle.IsValid());

    Subsystem->Tick(0.5f);
    TestEqual(TEXT("Slot Position at 50%"), Slot->GetPosition(), FVector2D(100.f, 150.f));
    TestEqual(TEXT("Slot Size at 50%"), Slot->GetSize(), FVector2D(300.f, 350.f));

    Subsystem->Tick(0.5f);
    TestEqual(TEXT("Slot Position at 100%"), Slot->GetPosition(), FVector2D(200.f, 300.f));
    TestEqual(TEXT("Slot Size at 100%"), Slot->GetSize(), FVector2D(500.f, 600.f));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenBlueprintHandleSettersTest,
    "UTween.Blueprint.HandleSetters", UTWEEN_TEST_FLAGS)

bool FUTweenBlueprintHandleSettersTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    float Val = 0.0f;
    auto UpdateFunc = [&Val](float V) { Val = V; };
    FTweenHandle RawHandle = UTween::Value(GI, 0.0f, 100.0f, 1.0f, UpdateFunc);
    FBPTweenHandle BPHandle(RawHandle);

    UTweenBlueprintLibrary::SetDelay(GI, BPHandle, 0.5f);
    UTweenBlueprintLibrary::SetLoops(GI, BPHandle, 3, ETweenLoopMode::Yoyo);
    UTweenBlueprintLibrary::SetEase(GI, BPHandle, ETweenEase::InCubic);
    UTweenBlueprintLibrary::SetRelative(GI, BPHandle, true);
    UTweenBlueprintLibrary::SetTimeMode(GI, BPHandle, ETweenTimeMode::RealTime);
    UTweenBlueprintLibrary::SetTag(GI, BPHandle, FName("TestTag"));

    FTweenHandle Resolved = BPHandle.Resolve(GI);
    TestTrue(TEXT("Resolved valid"), Resolved.IsValid());
    TSharedPtr<FTweenInstanceBase> Inst = Resolved.GetInstance();
    TestNotNull(TEXT("Inst valid"), Inst.Get());
    TestEqual(TEXT("Delay set"), Inst->Delay, 0.5f);
    TestEqual(TEXT("LoopCount set"), Inst->LoopCount, 3);
    TestEqual(TEXT("LoopMode set"), Inst->LoopMode, ETweenLoopMode::Yoyo);
    TestEqual(TEXT("Ease set"), Inst->Ease, ETweenEase::InCubic);
    TestTrue(TEXT("bIsRelative set"), Inst->bIsRelative);
    TestEqual(TEXT("TimeMode set"), Inst->TimeMode, ETweenTimeMode::RealTime);
    TestEqual(TEXT("Tag set"), Inst->Tag.Name, FName("TestTag"));

    // Test CompleteTween
    UTweenBlueprintLibrary::CompleteTween(GI, BPHandle);
    TestEqual(TEXT("Value reached 100 on complete"), Val, 100.f, FloatTolerance);
    TestTrue(TEXT("Tween auto-killed on complete"), Inst->IsKilled());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenBlueprintSequenceSettersTest,
    "UTween.Blueprint.SequenceSetters", UTWEEN_TEST_FLAGS)

bool FUTweenBlueprintSequenceSettersTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    FBPTweenSequenceHandle Seq1 = UTweenBlueprintLibrary::CreateTweenSequence(GI);
    FBPTweenSequenceHandle Seq2 = UTweenBlueprintLibrary::CreateTweenSequence(GI);

    UTweenBlueprintLibrary::SetSequenceLoops(GI, Seq1, 2, ETweenLoopMode::Restart);
    UTweenBlueprintLibrary::SetSequenceTimeMode(GI, Seq1, ETweenTimeMode::RealTime);
    UTweenBlueprintLibrary::SetSequenceTag(GI, Seq1, FName("SeqTag"));
    UTweenBlueprintLibrary::AppendSubSequence(GI, Seq1, Seq2);

    FTweenSequenceHandle Resolved1 = Seq1.Resolve(GI);
    TestTrue(TEXT("Seq1 valid"), Resolved1.IsValid());
    TSharedPtr<FTweenSequence> SeqInst = Resolved1.GetInstance();
    TestNotNull(TEXT("SeqInst valid"), SeqInst.Get());
    TestEqual(TEXT("Seq LoopCount set"), SeqInst->LoopCount, 2);
    TestEqual(TEXT("Seq LoopMode set"), SeqInst->LoopMode, ETweenLoopMode::Restart);
    TestEqual(TEXT("Seq TimeMode set"), SeqInst->TimeMode, ETweenTimeMode::RealTime);
    TestEqual(TEXT("Seq Tag set"), SeqInst->Tag.Name, FName("SeqTag"));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenJuiceWidgetPunchTest,
    "UTween.Juice.WidgetPunchPositionAndScale", UTWEEN_TEST_FLAGS)

bool FUTweenJuiceWidgetPunchTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    UImage* Widget = NewObject<UImage>(GI);
    Widget->SetRenderTranslation(FVector2D(100.f, 200.f));
    Widget->SetRenderScale(FVector2D(1.f, 1.f));

    FTweenHandle PunchPos = UTween::PunchPosition(Widget, FVector2D(50.f, 0.f), 1.0f);
    FTweenHandle PunchScale = UTween::PunchScale(Widget, FVector2D(0.5f, 0.5f), 1.0f);

    TestTrue(TEXT("PunchPos valid"), PunchPos.IsValid());
    TestTrue(TEXT("PunchScale valid"), PunchScale.IsValid());

    // Halfway tick (0.5s)
    Subsystem->Tick(0.5f);
    // Complete tick (1.0s) - values must be cleanly restored to initial baseline
    Subsystem->Tick(0.5f);

    TestEqual(TEXT("PunchPos cleanly restored baseline"), Widget->GetRenderTransform().Translation, FVector2D(100.f, 200.f));
    TestEqual(TEXT("PunchScale cleanly restored baseline"), Widget->GetRenderTransform().Scale, FVector2D(1.f, 1.f));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenJuiceWidgetShakeTest,
    "UTween.Juice.WidgetShakePosition", UTWEEN_TEST_FLAGS)

bool FUTweenJuiceWidgetShakeTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    UImage* Widget = NewObject<UImage>(GI);
    Widget->SetRenderTranslation(FVector2D(50.f, 50.f));

    FTweenHandle ShakeHandle = UTween::ShakePosition(Widget, 25.0f, 1.0f);
    TestTrue(TEXT("ShakeHandle valid"), ShakeHandle.IsValid());

    // During tick, shake displaces
    Subsystem->Tick(0.25f);
    TestTrue(TEXT("Widget moved during shake"), Widget->GetRenderTransform().Translation != FVector2D(50.f, 50.f));

    // Finish tween
    Subsystem->Tick(0.75f);
    TestEqual(TEXT("Widget restored to base position on complete"), Widget->GetRenderTransform().Translation, FVector2D(50.f, 50.f));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenActorPunchAndUniformScaleTest,
    "UTween.Actor.ActorPunchAndUniformScale", UTWEEN_TEST_FLAGS)

bool FUTweenActorPunchAndUniformScaleTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!World)
    {
        return true;
    }

    UGameInstance* GI = NewObject<UGameInstance>();
    World->SetGameInstance(GI);
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);

    AActor* Actor = World->SpawnActor<AActor>();
    if (!Actor)
    {
        World->DestroyWorld(false);
        return true;
    }

    USceneComponent* RootComp = NewObject<USceneComponent>(Actor);
    Actor->SetRootComponent(RootComp);
    RootComp->RegisterComponent();

    Actor->SetActorLocation(FVector(100.f, 200.f, 300.f));
    Actor->SetActorScale3D(FVector(1.f, 1.f, 1.f));

    // Test Uniform ActorScale
    FTweenHandle ScaleHandle = UTween::ActorScale(Actor, 2.0f, 1.0f).SetEase(ETweenEase::Linear);
    Subsystem->Tick(0.5f);
    TestEqual(TEXT("Actor uniform scale at 50%"), Actor->GetActorScale3D(), FVector(1.5f, 1.5f, 1.5f));
    Subsystem->Tick(0.5f);
    TestEqual(TEXT("Actor uniform scale at 100%"), Actor->GetActorScale3D(), FVector(2.0f, 2.0f, 2.0f));

    // Test Actor PunchPosition
    FTweenHandle PunchHandle = UTween::PunchPosition(Actor, FVector(0.f, 0.f, 100.f), 1.0f);
    Subsystem->Tick(0.5f);
    Subsystem->Tick(0.5f);
    TestEqual(TEXT("Actor location restored after punch"), Actor->GetActorLocation(), FVector(100.f, 200.f, 300.f));

    World->DestroyWorld(false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenRestartRelativeAndYoyoTest,
    "UTween.Tween.RestartPreservesRelativeAndYoyoOrientation", UTWEEN_TEST_FLAGS)

bool FUTweenRestartRelativeAndYoyoTest::RunTest(const FString& Parameters)
{
    float RelCaptured = 0.f;
    auto RelTween = MakeFloatTween(10.f, 5.f, 1.f, RelCaptured);
    RelTween->bIsRelative = true;

    RelTween->Tick(0.5f);
    TestEqual(TEXT("Relative at 50% is 12.5"), RelCaptured, 12.5f, FloatTolerance);

    RelTween->Restart();
    RelTween->Tick(1.0f);
    TestEqual(TEXT("Relative after Restart still lands on 15, not 20"), RelCaptured, 15.f, FloatTolerance);

    float YoyoCaptured = 0.f;
    auto YoyoTween = MakeFloatTween(0.f, 100.f, 1.f, YoyoCaptured);
    YoyoTween->LoopMode = ETweenLoopMode::Yoyo;
    YoyoTween->LoopCount = 2;

    YoyoTween->Tick(1.0f);
    TestEqual(TEXT("Yoyo end of forward cycle (loop 1)"), YoyoCaptured, 100.f, FloatTolerance);
    TestEqual(TEXT("CurrentLoop is 1 (odd)"), YoyoTween->CurrentLoop, 1);

    YoyoTween->Restart();
    YoyoTween->Tick(0.25f);
    TestEqual(TEXT("After Restart on odd yoyo loop, progresses forward (25%)"), YoyoCaptured, 25.f, FloatTolerance);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceYoyoRestartAndAutoPlayTest,
    "UTween.Sequence.YoyoRestartAndAutoPlay", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceYoyoRestartAndAutoPlayTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    float A = 0.f;
    float B = 0.f;
    auto T1 = MakeFloatTween(0.f, 100.f, 0.5f, A);
    auto T2 = MakeFloatTween(0.f, 200.f, 0.5f, B);

    FTweenSequenceHandle SeqHandle = UTween::Sequence(GI);
    SeqHandle.Append(FTweenHandle(T1))
             .Append(FTweenHandle(T2))
             .SetLoops(2, ETweenLoopMode::Yoyo);

    // Without calling SeqHandle.Play(), ticking the subsystem should auto-start the sequence
    Subsystem->Tick(1.0f);
    TestEqual(TEXT("End of forward sequence cycle: A=100"), A, 100.f, FloatTolerance);
    TestEqual(TEXT("End of forward sequence cycle: B=200"), B, 200.f, FloatTolerance);

    // Sequence is now in reverse cycle (CurrentLoop == 1). Restart it and verify forward execution.
    SeqHandle.Restart();
    A = 0.f;
    B = 0.f;
    Subsystem->Tick(0.25f);
    TestEqual(TEXT("After Restart on odd yoyo loop, T1 runs first (50%)"), A, 50.f, FloatTolerance);
    TestEqual(TEXT("T2 has not started yet"), B, 0.f, FloatTolerance);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenPunchDelayedAndCustomCallbacksTest,
    "UTween.Juice.PunchDelayedBaselineAndUserCallbacks", UTWEEN_TEST_FLAGS)

bool FUTweenPunchDelayedAndCustomCallbacksTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    UImage* Widget = NewObject<UImage>(GI);
    Widget->SetRenderTranslation(FVector2D(10.f, 20.f));

    bool bUserCompleteCalled = false;
    FTweenHandle Punch = UTween::PunchPosition(Widget, FVector2D(50.f, 0.f), 1.0f)
        .SetDelay(0.5f)
        .OnComplete([&bUserCompleteCalled]()
        {
            bUserCompleteCalled = true;
        });

    // Move widget during the delay before the punch starts
    Widget->SetRenderTranslation(FVector2D(300.f, 400.f));

    Subsystem->Tick(0.5f); // Consumes delay
    Subsystem->Tick(0.5f); // Halfway through punch
    Subsystem->Tick(0.5f); // Completes punch

    TestTrue(TEXT("User OnComplete callback fired"), bUserCompleteCalled);
    TestEqual(TEXT("Punch restored post-delay baseline (300, 400)"), Widget->GetRenderTransform().Translation, FVector2D(300.f, 400.f));

    // Also verify mid-flight Kill restores baseline and still fires user OnKill
    bool bUserKillCalled = false;
    FTweenHandle PunchToKill = UTween::PunchPosition(Widget, FVector2D(100.f, 0.f), 1.0f)
        .OnKill([&bUserKillCalled]()
        {
            bUserKillCalled = true;
        });

    Subsystem->Tick(0.25f);
    TestTrue(TEXT("Widget displaced during punch"), Widget->GetRenderTransform().Translation != FVector2D(300.f, 400.f));

    PunchToKill.Kill();
    TestTrue(TEXT("User OnKill callback fired"), bUserKillCalled);
    TestEqual(TEXT("Punch restored baseline on Kill"), Widget->GetRenderTransform().Translation, FVector2D(300.f, 400.f));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenSequenceChildPoolingAndIdLookupTest,
    "UTween.Subsystem.SequenceChildPoolingAndFastIdLookup", UTWEEN_TEST_FLAGS)

bool FUTweenSequenceChildPoolingAndIdLookupTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    float V = 0.f;
    FTweenHandle ChildHandle = UTween::Value(GI, 0.f, 100.f, 0.5f, [&V](float InV) { V = InV; });
    FTweenInstanceBase* OriginalPtr = ChildHandle.GetInstance().Get();
    TestNotNull(TEXT("Child instance allocated"), OriginalPtr);

    // Verify O(1) lookup while registered as standalone tween
    TestTrue(TEXT("FindTweenById resolves registered tween"), Subsystem->FindTweenById(ChildHandle.GetId()).IsValid());

    FTweenSequenceHandle SeqHandle = UTween::Sequence(GI);
    TestTrue(TEXT("FindSequenceById resolves registered sequence"), Subsystem->FindSequenceById(SeqHandle.GetId()).IsValid());

    SeqHandle.Append(ChildHandle);
    TestFalse(TEXT("Detached child tween removed from standalone ID map"), Subsystem->FindTweenById(ChildHandle.GetId()).IsValid());

    // Release test-local shared pointers via handle reset so pool is sole owner after completion
    ChildHandle = FTweenHandle();

    // Tick sequence to completion (0.5s), which recycles child tween back into FloatPool
    Subsystem->Tick(0.5f);
    TestEqual(TEXT("Child tween reached 100"), V, 100.f, FloatTolerance);
    TestFalse(TEXT("Completed sequence removed from SequenceById"), Subsystem->FindSequenceById(SeqHandle.GetId()).IsValid());

    // Next AcquireTween<float>() should return the recycled instance pointer from FloatPool
    TSharedPtr<TTweenInstance<float>> Reacquired = Subsystem->AcquireTween<float>();
    TestEqual(TEXT("Sequence child tween was recycled into FloatPool"), static_cast<FTweenInstanceBase*>(Reacquired.Get()), OriginalPtr);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUTweenPerformanceBenchmarkTest,
    "UTween.Benchmark.ThroughputSuite", UTWEEN_TEST_FLAGS)

bool FUTweenPerformanceBenchmarkTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    UTweenSubsystem* Subsystem = NewObject<UTweenSubsystem>(GI);
    TestNotNull(TEXT("Subsystem created"), Subsystem);

    constexpr int32 NumTweens = 10000;
    constexpr int32 NumFrames = 100;

    float DummyFloat = 0.f;
    FVector DummyVec = FVector::ZeroVector;
    FRotator DummyRot = FRotator::ZeroRotator;

    TArray<FTweenId> Ids;
    Ids.Reserve(NumTweens);

    // 1. Cold creation of 10,000 float tweens
    const double ColdCreateStart = FPlatformTime::Seconds();
    for (int32 i = 0; i < NumTweens; ++i)
    {
        FTweenHandle H = UTween::Value(GI, 0.f, 100.f, 1.0f, [&DummyFloat](float V) { DummyFloat = V; })
            .SetEase(ETweenEase::OutQuad);
        Ids.Add(H.GetId());
    }
    const double ColdCreateMs = (FPlatformTime::Seconds() - ColdCreateStart) * 1000.0;

    // 2. 10,000 ID lookups
    const double LookupStart = FPlatformTime::Seconds();
    int32 ValidCount = 0;
    for (int32 i = 0; i < NumTweens; ++i)
    {
        if (Subsystem->FindTweenById(Ids[i]).IsValid())
        {
            ++ValidCount;
        }
    }
    const double LookupMs = (FPlatformTime::Seconds() - LookupStart) * 1000.0;
    TestEqual(TEXT("All 10,000 IDs resolved"), ValidCount, NumTweens);

    // 3. Tick 10,000 active float tweens for 100 frames (1,000,000 tween evaluations)
    const double FloatTickStart = FPlatformTime::Seconds();
    for (int32 Frame = 0; Frame < NumFrames; ++Frame)
    {
        Subsystem->Tick(0.005f);
    }
    const double FloatTickTotalMs = (FPlatformTime::Seconds() - FloatTickStart) * 1000.0;

    // 4. Complete & sweep/recycle all 10,000 float tweens
    const double SweepStart = FPlatformTime::Seconds();
    Subsystem->Tick(1.0f);
    const double SweepMs = (FPlatformTime::Seconds() - SweepStart) * 1000.0;
    TestEqual(TEXT("All float tweens swept"), Subsystem->GetActiveTweenCount(), 0);

    // 5. Warm (pooled) creation of 10,000 float tweens
    const double WarmCreateStart = FPlatformTime::Seconds();
    for (int32 i = 0; i < NumTweens; ++i)
    {
        UTween::Value(GI, 0.f, 100.f, 1.0f, [&DummyFloat](float V) { DummyFloat = V; });
    }
    const double WarmCreateMs = (FPlatformTime::Seconds() - WarmCreateStart) * 1000.0;
    Subsystem->KillAll();

    // 6. Rotator Slerp benchmark: 10,000 rotator tweens ticked for 100 frames
    for (int32 i = 0; i < NumTweens; ++i)
    {
        UTween::Value(GI, FRotator(0.f, 0.f, 0.f), FRotator(45.f, 120.f, 30.f), 1.0f,
            [&DummyRot](const FRotator& R) { DummyRot = R; });
    }
    const double RotTickStart = FPlatformTime::Seconds();
    for (int32 Frame = 0; Frame < NumFrames; ++Frame)
    {
        Subsystem->Tick(0.005f);
    }
    const double RotTickTotalMs = (FPlatformTime::Seconds() - RotTickStart) * 1000.0;
    Subsystem->KillAll();

    // 7. Sequence build + tick benchmark: 1,000 sequences x 5 tweens = 5,000 sequenced tweens
    constexpr int32 NumSeqs = 1000;
    constexpr int32 TweensPerSeq = 5;
    const double SeqBuildStart = FPlatformTime::Seconds();
    for (int32 s = 0; s < NumSeqs; ++s)
    {
        FTweenSequenceHandle Seq = UTween::Sequence(GI);
        for (int32 t = 0; t < TweensPerSeq; ++t)
        {
            Seq.Append(UTween::Value(GI, 0.f, 100.f, 0.2f, [&DummyFloat](float V) { DummyFloat = V; }));
        }
    }
    const double SeqBuildMs = (FPlatformTime::Seconds() - SeqBuildStart) * 1000.0;

    const double SeqTickStart = FPlatformTime::Seconds();
    for (int32 Frame = 0; Frame < NumFrames; ++Frame)
    {
        Subsystem->Tick(0.005f);
    }
    const double SeqTickTotalMs = (FPlatformTime::Seconds() - SeqTickStart) * 1000.0;
    Subsystem->KillAll();

    UE_LOG(LogUTween, Display,
        TEXT("[UTween Benchmark] ColdCreate(10k)=%.3fms | WarmCreate(10k)=%.3fms | Lookup(10k)=%.3fms | FloatTick(10k/frame)=%.3fms | RotTick(10k/frame)=%.3fms | Sweep(10k)=%.3fms | SeqBuild(1k*5)=%.3fms | SeqTick(1k*5/frame)=%.3fms"),
        ColdCreateMs,
        WarmCreateMs,
        LookupMs,
        FloatTickTotalMs / NumFrames,
        RotTickTotalMs / NumFrames,
        SweepMs,
        SeqBuildMs,
        SeqTickTotalMs / NumFrames);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
