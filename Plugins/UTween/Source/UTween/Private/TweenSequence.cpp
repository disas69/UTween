#include "TweenSequence.h"
#include "TweenInstance.h"
#include "TweenHandle.h"
#include "UTween.h"
#include "UTweenSubsystem.h"
#include "UTweenModule.h"
#include "Algo/Reverse.h"

void FTweenSequence::Play()
{
    if (State == ETweenState::Completed || State == ETweenState::Killed)
    {
        return;
    }
    State = ETweenState::Playing;
}

void FTweenSequence::Pause()
{
    if (State == ETweenState::Playing || State == ETweenState::Pending)
    {
        State = ETweenState::Paused;
    }
}

void FTweenSequence::Resume()
{
    if (State == ETweenState::Paused)
    {
        State = ETweenState::Playing;
    }
}

void FTweenSequence::Kill()
{
    if (State == ETweenState::Killed)
    {
        return;
    }

    for (FSequenceAction& Action : Actions)
    {
        if (Action.Type == ESequenceActionType::Tween && Action.TweenInstance.IsValid())
        {
            Action.TweenInstance->Kill();
        }
    }

    State = ETweenState::Killed;

    if (OnKillCallback)
    {
        OnKillCallback();
    }
}

void FTweenSequence::Tick(float DeltaTime)
{
    if (State != ETweenState::Playing)
    {
        return;
    }

    if (bBoundTargetExplicit && !BoundTarget.IsValid())
    {
        Kill();
        return;
    }

    const float PrevCursor = Cursor;
    Cursor += DeltaTime;

    for (FSequenceAction& Action : Actions)
    {
        if (Action.bComplete)
        {
            continue;
        }

        const float ActionEnd = Action.StartTime + Action.Duration;

        if (!Action.bStarted && Cursor >= Action.StartTime)
        {
            Action.bStarted = true;

            if (Action.Type == ESequenceActionType::Callback)
            {
                if (Action.CallbackFunc)
                {
                    Action.CallbackFunc();
                }
                Action.bComplete = true;
                continue;
            }

            if (Action.Type == ESequenceActionType::Tween && Action.TweenInstance.IsValid())
            {
                Action.TweenInstance->State = ETweenState::Playing;
            }
        }

        if (Action.bStarted && Action.Type == ESequenceActionType::Tween && !Action.bComplete)
        {
            if (Action.TweenInstance.IsValid())
            {
                const float SliceStart = FMath::Max(PrevCursor, Action.StartTime);
                const float SliceEnd = FMath::Min(Cursor, ActionEnd);
                const float OverlapDelta = FMath::Max(0.f, SliceEnd - SliceStart);

                if (OverlapDelta > 0.f)
                {
                    Action.TweenInstance->Tick(OverlapDelta);
                }

                if (Action.TweenInstance->IsComplete() || Action.TweenInstance->IsKilled() || Cursor >= ActionEnd)
                {
                    if (!Action.TweenInstance->IsComplete() && !Action.TweenInstance->IsKilled())
                    {
                        Action.TweenInstance->Complete();
                    }
                    Action.bComplete = true;
                }
            }
            else
            {
                Action.bComplete = true;
            }
        }

        if (Action.Type == ESequenceActionType::Interval && Cursor >= ActionEnd)
        {
            Action.bComplete = true;
        }
    }

    if (AllActionsComplete())
    {
        HandleCompletion();
    }
}

bool FTweenSequence::AllActionsComplete() const
{
    for (const FSequenceAction& A : Actions)
    {
        if (!A.bComplete)
        {
            return false;
        }
    }
    return true;
}

void FTweenSequence::HandleCompletion()
{
    const bool bInfinite = (LoopMode != ETweenLoopMode::None) && (LoopCount < 0);
    const bool bHasMore = (LoopMode != ETweenLoopMode::None) && (LoopCount > 0) && (CurrentLoop < LoopCount);

    if (!bInfinite && !bHasMore)
    {
        State = ETweenState::Completed;
        if (OnCompleteCallback)
        {
            OnCompleteCallback();
        }
        return;
    }

    ++CurrentLoop;
    ResetForLoop();
}

void FTweenSequence::ResetForLoop()
{
    const float SafeTotalDuration = FMath::Max(TotalDuration, KINDA_SMALL_NUMBER);
    Cursor = FMath::Max(0.f, Cursor - SafeTotalDuration);
    const bool bYoyo = (LoopMode == ETweenLoopMode::Yoyo);

    if (bYoyo)
    {
        // Mirror the timeline: an action ending at T now starts at TotalDuration-T.
        // Reversing the array preserves parallel group cohesion.
        for (FSequenceAction& Action : Actions)
        {
            const float End = Action.StartTime + Action.Duration;
            Action.StartTime = FMath::Max(0.f, TotalDuration - End);
        }
        Algo::Reverse(Actions);
    }

    for (FSequenceAction& Action : Actions)
    {
        Action.bStarted = false;
        Action.bComplete = false;

        if (Action.Type == ESequenceActionType::Tween && Action.TweenInstance.IsValid())
        {
            FTweenInstanceBase* Inst = Action.TweenInstance.Get();
            Inst->Elapsed = 0.f;
            Inst->DelayElapsed = 0.f;
            Inst->CurrentLoop = 0;
            Inst->bStartFired = false;
            Inst->State = ETweenState::Pending;

            if (bYoyo)
            {
                Inst->SwapStartAndEnd();
            }
        }
    }
}

void FTweenSequence::AddTweenAction(TSharedPtr<FTweenInstanceBase> Instance, bool bJoin)
{
    if (!Instance.IsValid())
    {
        return;
    }

    // Sequence owns child lifecycle; suppress independent looping / auto-kill.
    Instance->LoopMode = ETweenLoopMode::None;
    Instance->LoopCount = 0;
    Instance->CurrentLoop = 0;
    Instance->bAutoKill = false;
    Instance->State = ETweenState::Pending;
    Instance->Elapsed = 0.f;
    Instance->DelayElapsed = 0.f;
    Instance->bStartFired = false;

    const float ActionDuration = FMath::Max(0.f, Instance->Duration + Instance->Delay);

    FSequenceAction Action;
    Action.Type = ESequenceActionType::Tween;
    Action.Duration = ActionDuration;
    Action.TweenInstance = Instance;

    if (bJoin && Actions.Num() > 0)
    {
        Action.StartTime = LastAppendStart;
    }
    else
    {
        Action.StartTime = InsertCursor;
        LastAppendStart = InsertCursor;
        LastAppendDuration = ActionDuration;
        InsertCursor += ActionDuration;
    }

    const float End = Action.StartTime + Action.Duration;
    TotalDuration = FMath::Max(TotalDuration, End);
    if (bJoin)
    {
        InsertCursor = FMath::Max(InsertCursor, End);
    }

    Actions.Add(MoveTemp(Action));
}

void FTweenSequence::AddIntervalAction(float InDuration)
{
    const float Safe = FMath::Max(0.f, InDuration);

    FSequenceAction Action;
    Action.Type = ESequenceActionType::Interval;
    Action.StartTime = InsertCursor;
    Action.Duration = Safe;

    LastAppendStart = InsertCursor;
    LastAppendDuration = Safe;
    InsertCursor += Safe;
    TotalDuration = FMath::Max(TotalDuration, Action.StartTime + Action.Duration);

    Actions.Add(MoveTemp(Action));
}

void FTweenSequence::AddCallbackAction(TFunction<void()> Callback)
{
    FSequenceAction Action;
    Action.Type = ESequenceActionType::Callback;
    Action.StartTime = InsertCursor;
    Action.Duration = 0.f;
    Action.CallbackFunc = MoveTemp(Callback);

    TotalDuration = FMath::Max(TotalDuration, Action.StartTime);

    Actions.Add(MoveTemp(Action));
}

FTweenSequenceHandle::FTweenSequenceHandle(TSharedPtr<FTweenSequence> InSequence)
    : SequencePtr(InSequence)
    , Id(InSequence.IsValid() ? InSequence->Id : FTweenId{})
{
}

FTweenSequence* FTweenSequenceHandle::Pin() const
{
    TSharedPtr<FTweenSequence> Pinned = SequencePtr.Pin();
    return Pinned.Get();
}

void FTweenSequenceHandle::AttachTweenHandle(FTweenHandle Tween, bool bJoin)
{
    FTweenSequence* Seq = Pin();
    if (!Seq)
    {
        return;
    }

    const FTweenId TweenId = Tween.GetId();
    if (!TweenId.IsValid())
    {
        return;
    }

    UTweenSubsystem* Subsystem = UTween::GetSubsystem();
    if (!Subsystem)
    {
        UE_LOG(LogUTween, Warning, TEXT("FTweenSequenceHandle::Attach: no active subsystem; ignoring."));
        return;
    }

    TSharedPtr<FTweenInstanceBase> Instance = Subsystem->DetachTween(TweenId);
    if (!Instance.IsValid())
    {
        UE_LOG(LogUTween, Warning,
            TEXT("FTweenSequenceHandle::Attach: tween Id=%lld not found on subsystem; ignoring."),
            TweenId.Value);
        return;
    }

    Seq->AddTweenAction(Instance, bJoin);
}

FTweenSequenceHandle& FTweenSequenceHandle::Append(FTweenHandle Tween)
{
    AttachTweenHandle(Tween, false);
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::Join(FTweenHandle Tween)
{
    AttachTweenHandle(Tween, true);
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::AppendInterval(float InDuration)
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->AddIntervalAction(InDuration);
    }
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::AppendCallback(TFunction<void()> Callback)
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->AddCallbackAction(MoveTemp(Callback));
    }
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::SetLoops(int32 Count, ETweenLoopMode Mode)
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->LoopCount = Count;
        Seq->LoopMode = (Count == 0) ? ETweenLoopMode::None : Mode;
    }
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::SetTimeMode(ETweenTimeMode Mode)
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->TimeMode = Mode;
    }
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::SetTag(FTweenTag InTag)
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->Tag = InTag;
    }
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::SetBoundTarget(UObject* Target)
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->BoundTarget = Target;
        Seq->bBoundTargetExplicit = (Target != nullptr);
    }
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::OnComplete(TFunction<void()> Callback)
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->OnCompleteCallback = MoveTemp(Callback);
    }
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::OnKill(TFunction<void()> Callback)
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->OnKillCallback = MoveTemp(Callback);
    }
    return *this;
}

FTweenSequenceHandle& FTweenSequenceHandle::Play()
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->Play();
    }
    return *this;
}

void FTweenSequenceHandle::Pause()
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->Pause();
    }
}

void FTweenSequenceHandle::Resume()
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->Resume();
    }
}

void FTweenSequenceHandle::Kill()
{
    if (FTweenSequence* Seq = Pin())
    {
        Seq->Kill();
    }
}

bool FTweenSequenceHandle::IsValid() const
{
    const FTweenSequence* Seq = Pin();
    return Seq != nullptr && !Seq->IsKilled();
}

bool FTweenSequenceHandle::IsPlaying() const
{
    const FTweenSequence* Seq = Pin();
    return Seq != nullptr && Seq->State == ETweenState::Playing;
}
