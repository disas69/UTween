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
        DelayElapsed += DeltaTime;
        if (DelayElapsed < Delay)
        {
            return;
        }
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
    Elapsed = 0.f;

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

bool FTweenInstanceBase::IsTargetValid() const
{
    // Default-constructed TWeakObjectPtr is not stale, so unbound tweens pass.
    return !BoundTarget.IsStale();
}
