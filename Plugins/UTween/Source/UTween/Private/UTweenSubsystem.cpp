#include "UTweenSubsystem.h"
#include "UTweenModule.h"
#include "TweenInstance.h"
#include "TweenSequence.h"
#include "TweenHandle.h"
#include "Engine/GameInstance.h"
#include "Misc/App.h"
#include "Stats/Stats.h"

DECLARE_STATS_GROUP(TEXT("UTween"), STATGROUP_UTween, STATCAT_Advanced);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Tweens"), STAT_UTween_ActiveTweens, STATGROUP_UTween);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Sequences"), STAT_UTween_ActiveSequences, STATGROUP_UTween);
DECLARE_CYCLE_STAT(TEXT("Tick"), STAT_UTween_Tick, STATGROUP_UTween);

void UTweenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    TickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
                                                                  [this](float DeltaTime) -> bool
                                                                  {
                                                                      Tick(DeltaTime);
                                                                      return true;
                                                                  }),
        0.0f);

    UE_LOG(LogUTween, Log, TEXT("UTweenSubsystem initialized"));
}

void UTweenSubsystem::Deinitialize()
{
    if (TickDelegateHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
        TickDelegateHandle.Reset();
    }

    // Kill sequences before tweens so OnKill callbacks fire while refs are live.
    for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
    {
        if (Seq.IsValid())
        {
            Seq->Kill();
        }
    }
    ActiveSequences.Reset();

    KillAll();

    UE_LOG(LogUTween, Log, TEXT("UTweenSubsystem deinitialized"));

    Super::Deinitialize();
}

UTweenSubsystem* UTweenSubsystem::Get(const UObject* WorldContext)
{
    if (!WorldContext)
    {
        return nullptr;
    }

    const UWorld* World = WorldContext->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        return nullptr;
    }

    return GI->GetSubsystem<UTweenSubsystem>();
}

TSharedPtr<FTweenInstanceBase> UTweenSubsystem::RegisterTween(TSharedPtr<FTweenInstanceBase> Instance)
{
    if (!Instance.IsValid())
    {
        return nullptr;
    }

    if (!Instance->Id.IsValid())
    {
        Instance->Id = FTweenId::Generate();
    }

    // Preserve Paused; otherwise force Pending so the tick loop advances to Playing next frame.
    if (Instance->State != ETweenState::Paused)
    {
        Instance->State = ETweenState::Pending;
    }

    ActiveTweens.Add(Instance);
    return Instance;
}

void UTweenSubsystem::RemoveTween(FTweenId Id)
{
    for (const TSharedPtr<FTweenInstanceBase>& Tween : ActiveTweens)
    {
        if (Tween.IsValid() && Tween->Id == Id)
        {
            Tween->Kill();
            break;
        }
    }
}

TSharedPtr<FTweenInstanceBase> UTweenSubsystem::DetachTween(FTweenId Id)
{
    for (int32 Index = 0; Index < ActiveTweens.Num(); ++Index)
    {
        const TSharedPtr<FTweenInstanceBase>& Tween = ActiveTweens[Index];
        if (Tween.IsValid() && Tween->Id == Id)
        {
            TSharedPtr<FTweenInstanceBase> Detached = Tween;
            if (bIsTicking)
            {
                ActiveTweens[Index] = nullptr;
            }
            else
            {
                ActiveTweens.RemoveAt(Index);
            }
            return Detached;
        }
    }
    return nullptr;
}

TSharedPtr<FTweenSequence> UTweenSubsystem::RegisterSequence(TSharedPtr<FTweenSequence> Sequence)
{
    if (!Sequence.IsValid())
    {
        return nullptr;
    }

    if (!Sequence->Id.IsValid())
    {
        Sequence->Id = FTweenId::Generate();
    }

    ActiveSequences.Add(Sequence);
    return Sequence;
}

void UTweenSubsystem::PauseAll()
{
    for (const TSharedPtr<FTweenInstanceBase>& Tween : ActiveTweens)
    {
        if (Tween.IsValid())
        {
            Tween->Pause();
        }
    }
    for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
    {
        if (Seq.IsValid())
        {
            Seq->Pause();
        }
    }
}

void UTweenSubsystem::ResumeAll()
{
    for (const TSharedPtr<FTweenInstanceBase>& Tween : ActiveTweens)
    {
        if (Tween.IsValid())
        {
            Tween->Resume();
        }
    }
    for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
    {
        if (Seq.IsValid())
        {
            Seq->Resume();
        }
    }
}

void UTweenSubsystem::KillAll()
{
    for (const TSharedPtr<FTweenInstanceBase>& Tween : ActiveTweens)
    {
        if (Tween.IsValid())
        {
            Tween->Kill();
        }
    }
    for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
    {
        if (Seq.IsValid())
        {
            Seq->Kill();
        }
    }

    if (!bIsTicking)
    {
        ActiveTweens.Reset();
        ActiveSequences.Reset();
    }
}

void UTweenSubsystem::PauseByTag(FTweenTag Tag)
{
    for (const TSharedPtr<FTweenInstanceBase>& Tween : ActiveTweens)
    {
        if (Tween.IsValid() && Tween->Tag == Tag)
        {
            Tween->Pause();
        }
    }
    for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
    {
        if (Seq.IsValid() && Seq->Tag == Tag)
        {
            Seq->Pause();
        }
    }
}

void UTweenSubsystem::ResumeByTag(FTweenTag Tag)
{
    for (const TSharedPtr<FTweenInstanceBase>& Tween : ActiveTweens)
    {
        if (Tween.IsValid() && Tween->Tag == Tag)
        {
            Tween->Resume();
        }
    }
    for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
    {
        if (Seq.IsValid() && Seq->Tag == Tag)
        {
            Seq->Resume();
        }
    }
}

void UTweenSubsystem::KillByTag(FTweenTag Tag)
{
    for (const TSharedPtr<FTweenInstanceBase>& Tween : ActiveTweens)
    {
        if (Tween.IsValid() && Tween->Tag == Tag)
        {
            Tween->Kill();
        }
    }
    for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
    {
        if (Seq.IsValid() && Seq->Tag == Tag)
        {
            Seq->Kill();
        }
    }
}

int32 UTweenSubsystem::GetActiveTweenCount() const
{
    return ActiveTweens.Num();
}

int32 UTweenSubsystem::GetActiveSequenceCount() const
{
    return ActiveSequences.Num();
}

FTweenHandle UTweenSubsystem::FindTweenById(FTweenId Id) const
{
    if (!Id.IsValid())
    {
        return FTweenHandle();
    }

    for (const TSharedPtr<FTweenInstanceBase>& Tween : ActiveTweens)
    {
        if (Tween.IsValid() && Tween->Id == Id)
        {
            return FTweenHandle(Tween);
        }
    }
    return FTweenHandle();
}

FTweenSequenceHandle UTweenSubsystem::FindSequenceById(FTweenId Id) const
{
    if (!Id.IsValid())
    {
        return FTweenSequenceHandle();
    }

    for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
    {
        if (Seq.IsValid() && Seq->Id == Id)
        {
            return FTweenSequenceHandle(Seq);
        }
    }
    return FTweenSequenceHandle();
}

void UTweenSubsystem::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_UTween_Tick);

    TGuardValue<bool> TickingGuard(bIsTicking, true);

    // FTSTicker delta is world-dilated (GameTime); FApp::GetDeltaTime() is unscaled (RealTime).
    const float GameDelta = DeltaTime;
    const float RealDelta = static_cast<float>(FApp::GetDeltaTime());

    // Snapshot count so newly-registered entries defer to the next frame.
    const int32 TweenCount = ActiveTweens.Num();
    for (int32 Index = 0; Index < TweenCount; ++Index)
    {
        TSharedPtr<FTweenInstanceBase> Tween = ActiveTweens[Index];
        if (!Tween.IsValid())
        {
            continue;
        }

        if (Tween->State == ETweenState::Pending)
        {
            Tween->State = ETweenState::Playing;
        }

        const float EffectiveDelta = (Tween->TimeMode == ETweenTimeMode::RealTime) ? RealDelta : GameDelta;
        Tween->Tick(EffectiveDelta);
    }

    const int32 SeqCount = ActiveSequences.Num();
    for (int32 Index = 0; Index < SeqCount; ++Index)
    {
        TSharedPtr<FTweenSequence> Seq = ActiveSequences[Index];
        if (!Seq.IsValid())
        {
            continue;
        }

        const float EffectiveDelta = (Seq->TimeMode == ETweenTimeMode::RealTime) ? RealDelta : GameDelta;
        Seq->Tick(EffectiveDelta);
    }

    ActiveTweens.RemoveAll([](const TSharedPtr<FTweenInstanceBase>& Tween)
        {
            return !Tween.IsValid() || Tween->State == ETweenState::Killed;
        });

    ActiveSequences.RemoveAll([](const TSharedPtr<FTweenSequence>& Seq)
        {
            return !Seq.IsValid() || Seq->State == ETweenState::Killed || Seq->State == ETweenState::Completed;
        });

    SET_DWORD_STAT(STAT_UTween_ActiveTweens, ActiveTweens.Num());
    SET_DWORD_STAT(STAT_UTween_ActiveSequences, ActiveSequences.Num());
}
