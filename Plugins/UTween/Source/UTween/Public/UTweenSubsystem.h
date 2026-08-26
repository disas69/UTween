#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "TweenTypes.h"
#include "UTweenSubsystem.generated.h"

class FTweenInstanceBase;
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

    /** Assign an Id, transition to Pending, and add the instance to the tick list. */
    TSharedPtr<FTweenInstanceBase> RegisterTween(TSharedPtr<FTweenInstanceBase> Instance);

    /** Mark a tween as killed so it will be swept on the next tick. */
    void RemoveTween(FTweenId Id);

    /** Remove a tween from the active list without killing it, transferring ownership. */
    TSharedPtr<FTweenInstanceBase> DetachTween(FTweenId Id);

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

    int32 GetActiveTweenCount() const;
    int32 GetActiveSequenceCount() const;
    /** Ticks all active tweens and sequences. Called by FTSTicker, or directly by automation tests. */
    void Tick(float DeltaTime);

private:
    FTSTicker::FDelegateHandle TickDelegateHandle;

    bool bIsTicking = false;

    TArray<TSharedPtr<FTweenInstanceBase>> ActiveTweens;
    TArray<TSharedPtr<FTweenSequence>> ActiveSequences;
};
