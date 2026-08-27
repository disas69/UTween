#include "TweenInstance.h"
#include "UTweenModule.h"

void FTweenInstanceBase::Tick(float DeltaTime)
{
    if (State != ETweenState::Playing)
    {
        return;
    }

    if (!IsTargetValid())
    {
        Kill();
        return;
    }

    if (DelayElapsed < Delay)
    {
        const float RemainingDelay = Delay - DelayElapsed;
        if (DeltaTime < RemainingDelay)
        {
            DelayElapsed += DeltaTime;
            return;
        }

        DelayElapsed = Delay;
        DeltaTime -= RemainingDelay;
    }

    if (!bStartFired)
    {
        if (!bUseFrom)
        {
            CaptureStartValue();
        }
        if (bIsRelative)
        {
            ApplyRelativeOffset();
        }
        bStartFired = true;

        if (OnStartCallback)
        {
            OnStartCallback();
        }
    }

    Elapsed += DeltaTime;

    const float SafeDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
    const float RawAlpha = FMath::Clamp(Elapsed / SafeDuration, 0.f, 1.f);
    const float Alpha = FTweenEasing::Evaluate(Ease, RawAlpha);

    EvaluateAndApply(Alpha);

    if (OnUpdateCallback)
    {
        OnUpdateCallback();
    }

    if (Elapsed >= Duration)
    {
        HandleLoopOrComplete();
    }
}

void FTweenInstanceBase::HandleLoopOrComplete()
{
    const bool bHasInfiniteLoops = (LoopMode != ETweenLoopMode::None) && (LoopCount < 0);
    const bool bLoopsRemaining = (LoopMode != ETweenLoopMode::None) && (LoopCount > 0) && (CurrentLoop < LoopCount);

    if (!bHasInfiniteLoops && !bLoopsRemaining)
    {
        Complete();
        return;
    }

    ++CurrentLoop;
    const float SafeDuration = FMath::Max(Duration, KINDA_SMALL_NUMBER);
    Elapsed = FMath::Max(0.f, Elapsed - SafeDuration);

    if (LoopMode == ETweenLoopMode::Yoyo)
    {
        SwapStartAndEnd();
    }
}

void FTweenInstanceBase::Complete()
{
    if (State == ETweenState::Completed || State == ETweenState::Killed)
    {
        return;
    }

    if (!bStartFired)
    {
        if (!bUseFrom)
        {
            CaptureStartValue();
        }
        if (bIsRelative)
        {
            ApplyRelativeOffset();
        }
        bStartFired = true;
    }

    EvaluateAndApply(1.f);
    State = ETweenState::Completed;

    UE_LOG(LogUTween, Verbose, TEXT("Tween completed (Id=%lld, Name=%s)"), Id.Value, *DebugName.ToString());

    if (OnCompleteCallback)
    {
        OnCompleteCallback();
    }

    if (bAutoKill)
    {
        Kill();
    }
}

void FTweenInstanceBase::Kill()
{
    if (State == ETweenState::Killed)
    {
        return;
    }

    State = ETweenState::Killed;

    UE_LOG(LogUTween, Verbose, TEXT("Tween killed (Id=%lld, Name=%s)"), Id.Value, *DebugName.ToString());

    if (OnKillCallback)
    {
        OnKillCallback();
    }
}

void FTweenInstanceBase::Pause()
{
    if (State == ETweenState::Playing || State == ETweenState::Pending)
    {
        State = ETweenState::Paused;
    }
}

void FTweenInstanceBase::Resume()
{
    if (State == ETweenState::Paused)
    {
        State = ETweenState::Playing;
    }
}

void FTweenInstanceBase::Play()
{
    if (State != ETweenState::Killed)
    {
        State = ETweenState::Playing;
    }
}

void FTweenInstanceBase::Restart()
{
    if (State == ETweenState::Killed)
    {
        return;
    }

    Elapsed = 0.f;
    DelayElapsed = 0.f;
    CurrentLoop = 0;
    bStartFired = false;
    State = ETweenState::Playing;
}

bool FTweenInstanceBase::IsTargetValid() const
{
    if (!bHasTarget)
    {
        return true;
    }
    return BoundTarget.IsValid();
}

void FTweenInstanceBase::ResetForPool()
{
    Id = FTweenId{};
    Tag = FTweenTag();
    DebugName = NAME_None;
    Duration = 0.f;
    Elapsed = 0.f;
    Delay = 0.f;
    DelayElapsed = 0.f;
    Ease = ETweenEase::OutQuad;
    LoopMode = ETweenLoopMode::None;
    LoopCount = 0;
    CurrentLoop = 0;
    bAutoKill = true;
    bUseFrom = false;
    bIsRelative = false;
    TimeMode = ETweenTimeMode::GameTime;
    State = ETweenState::Killed;
    bStartFired = false;
    bHasTarget = false;
    BoundTarget = nullptr;
    OwningSubsystem = nullptr;
    OnStartCallback = nullptr;
    OnUpdateCallback = nullptr;
    OnCompleteCallback = nullptr;
    OnKillCallback = nullptr;
}
