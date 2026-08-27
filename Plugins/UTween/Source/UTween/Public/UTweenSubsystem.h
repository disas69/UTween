#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "TweenTypes.h"
#include "TweenInstance.h"
#include "UTweenSubsystem.generated.h"

class FTweenSequence;
struct FTweenHandle;
struct FTweenSequenceHandle;

UCLASS()
class UTWEEN_API UTweenSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    static UTweenSubsystem* Get(const UObject* WorldContext);

    /** Acquire a pooled tween instance for the given type, or allocate a new one if pool is empty. */
    template <typename T>
    TSharedPtr<TTweenInstance<T>> AcquireTween();

    /** Recycle a dead tween instance back into the appropriate pool. */
    void RecycleTween(const TSharedPtr<FTweenInstanceBase>& Instance);

    /** Empty all instance pools. */
    void EmptyPools();

    /** Assign an Id, transition to Pending, and add the instance to the tick list. */
    TSharedPtr<FTweenInstanceBase> RegisterTween(TSharedPtr<FTweenInstanceBase> Instance);

    /** Mark a tween as killed so it will be swept on the next tick. */
    void RemoveTween(FTweenId Id);

    /** Remove a tween from the active list without killing it, transferring ownership. */
    TSharedPtr<FTweenInstanceBase> DetachTween(FTweenId Id);

    /** Remove a sequence from the active list without killing it, transferring ownership. */
    TSharedPtr<FTweenSequence> DetachSequence(FTweenId Id);

    /** Assign an Id and add the sequence to the tick list. */
    TSharedPtr<FTweenSequence> RegisterSequence(TSharedPtr<FTweenSequence> Sequence);

    /** Returns an invalid handle on miss. */
    FTweenHandle FindTweenById(FTweenId Id) const;

    /** Returns an invalid handle on miss. */
    FTweenSequenceHandle FindSequenceById(FTweenId Id) const;

    void PauseAll();
    void ResumeAll();
    void KillAll();

    void PauseByTag(FTweenTag Tag);
    void ResumeByTag(FTweenTag Tag);
    void KillByTag(FTweenTag Tag);

    void RegisterTweenTag(FTweenTag Tag, FTweenInstanceBase* Tween);
    void UnregisterTweenTag(FTweenTag Tag, FTweenInstanceBase* Tween);
    void RegisterSequenceTag(FTweenTag Tag, FTweenSequence* Sequence);
    void UnregisterSequenceTag(FTweenTag Tag, FTweenSequence* Sequence);

    int32 GetActiveTweenCount() const;
    int32 GetActiveSequenceCount() const;
    /** Ticks all active tweens and sequences. Called by FTSTicker, or directly by automation tests. */
    void Tick(float DeltaTime);

private:
    FTSTicker::FDelegateHandle TickDelegateHandle;

    bool bIsTicking = false;

    TArray<TSharedPtr<FTweenInstanceBase>> ActiveTweens;
    TArray<TSharedPtr<FTweenSequence>> ActiveSequences;

    TMap<FTweenTag, TArray<FTweenInstanceBase*>> TaggedTweens;
    TMap<FTweenTag, TArray<FTweenSequence*>> TaggedSequences;

    TArray<TSharedPtr<TTweenInstance<float>>> FloatPool;
    TArray<TSharedPtr<TTweenInstance<FVector2D>>> Vector2DPool;
    TArray<TSharedPtr<TTweenInstance<FVector>>> VectorPool;
    TArray<TSharedPtr<TTweenInstance<FRotator>>> RotatorPool;
    TArray<TSharedPtr<TTweenInstance<FLinearColor>>> LinearColorPool;
};

template <> UTWEEN_API TSharedPtr<TTweenInstance<float>> UTweenSubsystem::AcquireTween<float>();
template <> UTWEEN_API TSharedPtr<TTweenInstance<FVector2D>> UTweenSubsystem::AcquireTween<FVector2D>();
template <> UTWEEN_API TSharedPtr<TTweenInstance<FVector>> UTweenSubsystem::AcquireTween<FVector>();
template <> UTWEEN_API TSharedPtr<TTweenInstance<FRotator>> UTweenSubsystem::AcquireTween<FRotator>();
template <> UTWEEN_API TSharedPtr<TTweenInstance<FLinearColor>> UTweenSubsystem::AcquireTween<FLinearColor>();

template <typename T>
TSharedPtr<TTweenInstance<T>> UTweenSubsystem::AcquireTween()
{
    TSharedPtr<TTweenInstance<T>> NewInstance = MakeShared<TTweenInstance<T>>();
    NewInstance->Id = FTweenId::Generate();
    return NewInstance;
}
