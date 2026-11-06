#include "UTweenSubsystem.h"
#include "UTweenModule.h"
#include "TweenInstance.h"
#include "TweenSequence.h"
#include "TweenHandle.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Misc/App.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

DECLARE_STATS_GROUP(TEXT("UTween"), STATGROUP_UTween, STATCAT_Advanced);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Tweens"), STAT_UTween_ActiveTweens, STATGROUP_UTween);
DECLARE_DWORD_COUNTER_STAT(TEXT("Active Sequences"), STAT_UTween_ActiveSequences, STATGROUP_UTween);
DECLARE_CYCLE_STAT(TEXT("Tick"), STAT_UTween_Tick, STATGROUP_UTween);

TWeakObjectPtr<UTweenSubsystem> UTweenSubsystem::LastActiveSubsystem = nullptr;

UTweenSubsystem* UTweenSubsystem::GetLastActiveSubsystem()
{
    return LastActiveSubsystem.Get();
}

void UTweenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LastActiveSubsystem = this;

    ActiveTweens.Reserve(256);
    ActiveSequences.Reserve(64);
    TweenById.Reserve(256);
    SequenceById.Reserve(64);
    FloatPool.Reserve(128);
    VectorPool.Reserve(64);

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

    if (LastActiveSubsystem.Get() == this)
    {
        LastActiveSubsystem.Reset();
    }

    // Kill sequences before tweens so OnKill callbacks fire while refs are live.
    for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
    {
        if (Seq.IsValid())
        {
            Seq->Kill();
            RecycleSequenceChildren(*Seq);
        }
    }
    ActiveSequences.Reset();
    SequenceById.Reset();

    KillAll();
    EmptyPools();

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

    LastActiveSubsystem = this;

    if (!Instance->Id.IsValid())
    {
        Instance->Id = FTweenId::Generate();
    }

    // Preserve Paused; otherwise force Pending so the tick loop advances to Playing next frame.
    if (Instance->State != ETweenState::Paused)
    {
        Instance->State = ETweenState::Pending;
    }

    Instance->OwningSubsystem = this;
    Instance->ActiveIndex = ActiveTweens.Add(Instance);
    TweenById.Add(Instance->Id, Instance.Get());

    if (Instance->Tag.IsValid())
    {
        RegisterTweenTag(Instance->Tag, Instance.Get());
    }

    return Instance;
}

void UTweenSubsystem::RemoveTween(FTweenId Id)
{
    if (FTweenInstanceBase* const* Found = TweenById.Find(Id))
    {
        if (*Found)
        {
            (*Found)->Kill();
        }
    }
}

TSharedPtr<FTweenInstanceBase> UTweenSubsystem::DetachTween(FTweenId Id)
{
    FTweenInstanceBase* RawPtr = nullptr;
    if (!TweenById.RemoveAndCopyValue(Id, RawPtr) || !RawPtr)
    {
        return nullptr;
    }

    if (RawPtr->Tag.IsValid())
    {
        UnregisterTweenTag(RawPtr->Tag, RawPtr);
    }

    const int32 Index = RawPtr->ActiveIndex;
    RawPtr->ActiveIndex = INDEX_NONE;

    if (ActiveTweens.IsValidIndex(Index) && ActiveTweens[Index].Get() == RawPtr)
    {
        TSharedPtr<FTweenInstanceBase> Detached = MoveTemp(ActiveTweens[Index]);
        if (!bIsTicking)
        {
            ActiveTweens.RemoveAtSwap(Index, 1, EAllowShrinking::No);
            if (ActiveTweens.IsValidIndex(Index) && ActiveTweens[Index].IsValid())
            {
                ActiveTweens[Index]->ActiveIndex = Index;
            }
        }
        return Detached;
    }

    return nullptr;
}

TSharedPtr<FTweenSequence> UTweenSubsystem::DetachSequence(FTweenId Id)
{
    FTweenSequence* RawPtr = nullptr;
    if (!SequenceById.RemoveAndCopyValue(Id, RawPtr) || !RawPtr)
    {
        return nullptr;
    }

    if (RawPtr->Tag.IsValid())
    {
        UnregisterSequenceTag(RawPtr->Tag, RawPtr);
    }

    const int32 Index = RawPtr->ActiveIndex;
    RawPtr->ActiveIndex = INDEX_NONE;

    if (ActiveSequences.IsValidIndex(Index) && ActiveSequences[Index].Get() == RawPtr)
    {
        TSharedPtr<FTweenSequence> Detached = MoveTemp(ActiveSequences[Index]);
        if (!bIsTicking)
        {
            ActiveSequences.RemoveAtSwap(Index, 1, EAllowShrinking::No);
            if (ActiveSequences.IsValidIndex(Index) && ActiveSequences[Index].IsValid())
            {
                ActiveSequences[Index]->ActiveIndex = Index;
            }
        }
        return Detached;
    }

    return nullptr;
}

TSharedPtr<FTweenSequence> UTweenSubsystem::RegisterSequence(TSharedPtr<FTweenSequence> Sequence)
{
    if (!Sequence.IsValid())
    {
        return nullptr;
    }

    LastActiveSubsystem = this;

    if (!Sequence->Id.IsValid())
    {
        Sequence->Id = FTweenId::Generate();
    }

    Sequence->OwningSubsystem = this;
    Sequence->ActiveIndex = ActiveSequences.Add(Sequence);
    SequenceById.Add(Sequence->Id, Sequence.Get());

    if (Sequence->Tag.IsValid())
    {
        RegisterSequenceTag(Sequence->Tag, Sequence.Get());
    }

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
        for (const TSharedPtr<FTweenInstanceBase>& Tween : ActiveTweens)
        {
            RecycleTween(Tween);
        }
        for (const TSharedPtr<FTweenSequence>& Seq : ActiveSequences)
        {
            if (Seq.IsValid())
            {
                RecycleSequenceChildren(*Seq);
            }
        }
        ActiveTweens.Reset();
        ActiveSequences.Reset();
        TweenById.Reset();
        SequenceById.Reset();
        TaggedTweens.Reset();
        TaggedSequences.Reset();
    }
}

void UTweenSubsystem::PauseByTag(FTweenTag Tag)
{
    if (!Tag.IsValid())
    {
        return;
    }

    if (TArray<FTweenInstanceBase*>* Bucket = TaggedTweens.Find(Tag))
    {
        for (FTweenInstanceBase* Tween : *Bucket)
        {
            if (Tween && !Tween->IsKilled())
            {
                Tween->Pause();
            }
        }
    }

    if (TArray<FTweenSequence*>* SeqBucket = TaggedSequences.Find(Tag))
    {
        for (FTweenSequence* Seq : *SeqBucket)
        {
            if (Seq && !Seq->IsKilled())
            {
                Seq->Pause();
            }
        }
    }
}

void UTweenSubsystem::ResumeByTag(FTweenTag Tag)
{
    if (!Tag.IsValid())
    {
        return;
    }

    if (TArray<FTweenInstanceBase*>* Bucket = TaggedTweens.Find(Tag))
    {
        for (FTweenInstanceBase* Tween : *Bucket)
        {
            if (Tween && !Tween->IsKilled())
            {
                Tween->Resume();
            }
        }
    }

    if (TArray<FTweenSequence*>* SeqBucket = TaggedSequences.Find(Tag))
    {
        for (FTweenSequence* Seq : *SeqBucket)
        {
            if (Seq && !Seq->IsKilled())
            {
                Seq->Resume();
            }
        }
    }
}

void UTweenSubsystem::KillByTag(FTweenTag Tag)
{
    if (!Tag.IsValid())
    {
        return;
    }

    if (TArray<FTweenInstanceBase*>* Bucket = TaggedTweens.Find(Tag))
    {
        TArray<FTweenInstanceBase*> TweensToKill = *Bucket;
        for (FTweenInstanceBase* Tween : TweensToKill)
        {
            if (Tween && !Tween->IsKilled())
            {
                Tween->Kill();
            }
        }
    }

    if (TArray<FTweenSequence*>* SeqBucket = TaggedSequences.Find(Tag))
    {
        TArray<FTweenSequence*> SeqsToKill = *SeqBucket;
        for (FTweenSequence* Seq : SeqsToKill)
        {
            if (Seq && !Seq->IsKilled())
            {
                Seq->Kill();
            }
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

    if (FTweenInstanceBase* const* Found = TweenById.Find(Id))
    {
        const FTweenInstanceBase* RawPtr = *Found;
        if (RawPtr && ActiveTweens.IsValidIndex(RawPtr->ActiveIndex))
        {
            const TSharedPtr<FTweenInstanceBase>& Shared = ActiveTweens[RawPtr->ActiveIndex];
            if (Shared.Get() == RawPtr)
            {
                return FTweenHandle(Shared);
            }
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

    if (FTweenSequence* const* Found = SequenceById.Find(Id))
    {
        const FTweenSequence* RawPtr = *Found;
        if (RawPtr && ActiveSequences.IsValidIndex(RawPtr->ActiveIndex))
        {
            const TSharedPtr<FTweenSequence>& Shared = ActiveSequences[RawPtr->ActiveIndex];
            if (Shared.Get() == RawPtr)
            {
                return FTweenSequenceHandle(Shared);
            }
        }
    }
    return FTweenSequenceHandle();
}

void UTweenSubsystem::Tick(float DeltaTime)
{
    if (ActiveTweens.IsEmpty() && ActiveSequences.IsEmpty())
    {
        return;
    }

    TRACE_CPUPROFILER_EVENT_SCOPE(UTween_SubsystemTick);
    SCOPE_CYCLE_COUNTER(STAT_UTween_Tick);

    TGuardValue<bool> TickingGuard(bIsTicking, true);

    const UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;

    float GameDelta = DeltaTime;
    if (World)
    {
        if (World->IsPaused())
        {
            GameDelta = 0.f;
        }
        else if (World->GetDeltaSeconds() > 0.f)
        {
            GameDelta = World->GetDeltaSeconds();
        }
    }

    const float AppDelta = static_cast<float>(FApp::GetDeltaTime());
    const float RealDelta = (AppDelta > 0.f) ? AppDelta : DeltaTime;

    // Snapshot count so newly-registered entries defer to the next frame.
    const int32 TweenCount = ActiveTweens.Num();
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(UTween_TickTweens);
        for (int32 Index = 0; Index < TweenCount; ++Index)
        {
            FTweenInstanceBase* Tween = ActiveTweens[Index].Get();
            if (!Tween)
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
    }

    const int32 SeqCount = ActiveSequences.Num();
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(UTween_TickSequences);
        for (int32 Index = 0; Index < SeqCount; ++Index)
        {
            FTweenSequence* Seq = ActiveSequences[Index].Get();
            if (!Seq)
            {
                continue;
            }

            if (Seq->State == ETweenState::Pending && Seq->Actions.Num() > 0)
            {
                Seq->State = ETweenState::Playing;
            }

            const float EffectiveDelta = (Seq->TimeMode == ETweenTimeMode::RealTime) ? RealDelta : GameDelta;
            Seq->Tick(EffectiveDelta);
        }
    }

    {
        TRACE_CPUPROFILER_EVENT_SCOPE(UTween_Cleanup);
        for (int32 Index = ActiveTweens.Num() - 1; Index >= 0; --Index)
        {
            FTweenInstanceBase* Tween = ActiveTweens[Index].Get();
            if (!Tween || Tween->State == ETweenState::Killed)
            {
                if (Tween)
                {
                    TweenById.Remove(Tween->Id);
                    RecycleTween(ActiveTweens[Index]);
                }
                ActiveTweens.RemoveAtSwap(Index, 1, EAllowShrinking::No);
                if (ActiveTweens.IsValidIndex(Index) && ActiveTweens[Index].IsValid())
                {
                    ActiveTweens[Index]->ActiveIndex = Index;
                }
            }
        }

        for (int32 Index = ActiveSequences.Num() - 1; Index >= 0; --Index)
        {
            FTweenSequence* Seq = ActiveSequences[Index].Get();
            const bool bDead = !Seq || Seq->State == ETweenState::Killed || Seq->State == ETweenState::Completed;
            if (bDead)
            {
                if (Seq)
                {
                    SequenceById.Remove(Seq->Id);
                    Seq->ActiveIndex = INDEX_NONE;
                    if (Seq->Tag.IsValid())
                    {
                        UnregisterSequenceTag(Seq->Tag, Seq);
                    }
                    RecycleSequenceChildren(*Seq);
                }
                ActiveSequences.RemoveAtSwap(Index, 1, EAllowShrinking::No);
                if (ActiveSequences.IsValidIndex(Index) && ActiveSequences[Index].IsValid())
                {
                    ActiveSequences[Index]->ActiveIndex = Index;
                }
            }
        }
    }

    SET_DWORD_STAT(STAT_UTween_ActiveTweens, ActiveTweens.Num());
    SET_DWORD_STAT(STAT_UTween_ActiveSequences, ActiveSequences.Num());
}

void UTweenSubsystem::RecycleSequenceChildren(FTweenSequence& Sequence)
{
    for (FSequenceAction& Action : Sequence.Actions)
    {
        if (Action.Type == ESequenceActionType::Tween && Action.TweenInstance.IsValid())
        {
            RecycleTween(Action.TweenInstance);
            Action.TweenInstance.Reset();
        }
        else if (Action.Type == ESequenceActionType::SubSequence && Action.SubSequence.IsValid())
        {
            RecycleSequenceChildren(*Action.SubSequence);
            Action.SubSequence.Reset();
        }
    }
    Sequence.Actions.Reset();
}

void UTweenSubsystem::EmptyPools()
{
    FloatPool.Reset();
    Vector2DPool.Reset();
    VectorPool.Reset();
    RotatorPool.Reset();
    LinearColorPool.Reset();
    TweenById.Reset();
    SequenceById.Reset();
    TaggedTweens.Reset();
    TaggedSequences.Reset();
}

void UTweenSubsystem::RegisterTweenTag(FTweenTag Tag, FTweenInstanceBase* Tween)
{
    if (!Tag.IsValid() || !Tween)
    {
        return;
    }
    TaggedTweens.FindOrAdd(Tag).AddUnique(Tween);
}

void UTweenSubsystem::UnregisterTweenTag(FTweenTag Tag, FTweenInstanceBase* Tween)
{
    if (!Tag.IsValid() || !Tween)
    {
        return;
    }
    if (TArray<FTweenInstanceBase*>* Bucket = TaggedTweens.Find(Tag))
    {
        Bucket->RemoveSwap(Tween);
        if (Bucket->Num() == 0)
        {
            TaggedTweens.Remove(Tag);
        }
    }
}

void UTweenSubsystem::RegisterSequenceTag(FTweenTag Tag, FTweenSequence* Sequence)
{
    if (!Tag.IsValid() || !Sequence)
    {
        return;
    }
    TaggedSequences.FindOrAdd(Tag).AddUnique(Sequence);
}

void UTweenSubsystem::UnregisterSequenceTag(FTweenTag Tag, FTweenSequence* Sequence)
{
    if (!Tag.IsValid() || !Sequence)
    {
        return;
    }
    if (TArray<FTweenSequence*>* Bucket = TaggedSequences.Find(Tag))
    {
        Bucket->RemoveSwap(Sequence);
        if (Bucket->Num() == 0)
        {
            TaggedSequences.Remove(Tag);
        }
    }
}

void UTweenSubsystem::RecycleTween(const TSharedPtr<FTweenInstanceBase>& Instance)
{
    if (!Instance.IsValid())
    {
        return;
    }

    if (Instance->Tag.IsValid())
    {
        UnregisterTweenTag(Instance->Tag, Instance.Get());
    }

    Instance->ResetForPool();

    constexpr int32 MaxPoolCapacity = 16384;

    switch (Instance->ValueType)
    {
    case ETweenValueType::Float:
        if (FloatPool.Num() < MaxPoolCapacity)
        {
            FloatPool.Add(StaticCastSharedPtr<TTweenInstance<float>>(Instance));
        }
        break;
    case ETweenValueType::Vector2D:
        if (Vector2DPool.Num() < MaxPoolCapacity)
        {
            Vector2DPool.Add(StaticCastSharedPtr<TTweenInstance<FVector2D>>(Instance));
        }
        break;
    case ETweenValueType::Vector:
        if (VectorPool.Num() < MaxPoolCapacity)
        {
            VectorPool.Add(StaticCastSharedPtr<TTweenInstance<FVector>>(Instance));
        }
        break;
    case ETweenValueType::Rotator:
        if (RotatorPool.Num() < MaxPoolCapacity)
        {
            RotatorPool.Add(StaticCastSharedPtr<TTweenInstance<FRotator>>(Instance));
        }
        break;
    case ETweenValueType::LinearColor:
        if (LinearColorPool.Num() < MaxPoolCapacity)
        {
            LinearColorPool.Add(StaticCastSharedPtr<TTweenInstance<FLinearColor>>(Instance));
        }
        break;
    default:
        break;
    }
}

template <>
UTWEEN_API TSharedPtr<TTweenInstance<float>> UTweenSubsystem::AcquireTween<float>()
{
    if (FloatPool.Num() > 0)
    {
        TSharedPtr<TTweenInstance<float>> Pooled = FloatPool.Pop(EAllowShrinking::No);
        Pooled->Id = FTweenId::Generate();
        Pooled->State = ETweenState::Pending;
        return Pooled;
    }
    TSharedPtr<TTweenInstance<float>> NewInst = MakeShared<TTweenInstance<float>>();
    NewInst->Id = FTweenId::Generate();
    return NewInst;
}

template <>
UTWEEN_API TSharedPtr<TTweenInstance<FVector2D>> UTweenSubsystem::AcquireTween<FVector2D>()
{
    if (Vector2DPool.Num() > 0)
    {
        TSharedPtr<TTweenInstance<FVector2D>> Pooled = Vector2DPool.Pop(EAllowShrinking::No);
        Pooled->Id = FTweenId::Generate();
        Pooled->State = ETweenState::Pending;
        return Pooled;
    }
    TSharedPtr<TTweenInstance<FVector2D>> NewInst = MakeShared<TTweenInstance<FVector2D>>();
    NewInst->Id = FTweenId::Generate();
    return NewInst;
}

template <>
UTWEEN_API TSharedPtr<TTweenInstance<FVector>> UTweenSubsystem::AcquireTween<FVector>()
{
    if (VectorPool.Num() > 0)
    {
        TSharedPtr<TTweenInstance<FVector>> Pooled = VectorPool.Pop(EAllowShrinking::No);
        Pooled->Id = FTweenId::Generate();
        Pooled->State = ETweenState::Pending;
        return Pooled;
    }
    TSharedPtr<TTweenInstance<FVector>> NewInst = MakeShared<TTweenInstance<FVector>>();
    NewInst->Id = FTweenId::Generate();
    return NewInst;
}

template <>
UTWEEN_API TSharedPtr<TTweenInstance<FRotator>> UTweenSubsystem::AcquireTween<FRotator>()
{
    if (RotatorPool.Num() > 0)
    {
        TSharedPtr<TTweenInstance<FRotator>> Pooled = RotatorPool.Pop(EAllowShrinking::No);
        Pooled->Id = FTweenId::Generate();
        Pooled->State = ETweenState::Pending;
        return Pooled;
    }
    TSharedPtr<TTweenInstance<FRotator>> NewInst = MakeShared<TTweenInstance<FRotator>>();
    NewInst->Id = FTweenId::Generate();
    return NewInst;
}

template <>
UTWEEN_API TSharedPtr<TTweenInstance<FLinearColor>> UTweenSubsystem::AcquireTween<FLinearColor>()
{
    if (LinearColorPool.Num() > 0)
    {
        TSharedPtr<TTweenInstance<FLinearColor>> Pooled = LinearColorPool.Pop(EAllowShrinking::No);
        Pooled->Id = FTweenId::Generate();
        Pooled->State = ETweenState::Pending;
        return Pooled;
    }
    TSharedPtr<TTweenInstance<FLinearColor>> NewInst = MakeShared<TTweenInstance<FLinearColor>>();
    NewInst->Id = FTweenId::Generate();
    return NewInst;
}
