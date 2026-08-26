#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "TweenInstance.h"
#include "TweenSequence.h"

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

    // Cursor lands exactly at T1 end; T1 completes and T2 starts + ticks 0.5 in same frame.
    Seq->Tick(0.5f);
    TestEqual(TEXT("T1 complete"), A, 100.f, FloatTolerance);
    TestEqual(TEXT("T2 started and advanced"), B, 50.f, FloatTolerance);

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

    // Cursor crosses interval end at Tick(0.05); tween also ticks 0.05 in same frame.
    Seq->Tick(0.55f);
    TestEqual(TEXT("Tween started after interval"), A, 55.f, FloatTolerance);
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

#endif // WITH_DEV_AUTOMATION_TESTS
