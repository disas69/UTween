#pragma once

#include "CoreMinimal.h"
#include "TweenTypes.h"
#include "UObject/WeakObjectPtr.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"

class FTweenInstanceBase;
class FTweenSequence;
class UTweenSubsystem;
struct FTweenHandle;

enum class ESequenceActionType : uint8
{
    Tween,
    Interval,
    Callback,
    SubSequence,
};

/** One entry on a sequence's timeline. Flat tagged-union keyed off Type. */
struct FSequenceAction
{
    ESequenceActionType Type = ESequenceActionType::Interval;
    float StartTime = 0.f;
    float Duration = 0.f;

    TSharedPtr<FTweenInstanceBase> TweenInstance;
    TSharedPtr<FTweenSequence> SubSequence;
    TFunction<void()> CallbackFunc;

    bool bStarted = false;
    bool bComplete = false;
};

/** Runtime sequence: schedules child tweens, intervals, and callbacks on a timeline. */
class UTWEEN_API FTweenSequence : public TSharedFromThis<FTweenSequence>
{
public:
    FTweenId Id;
    FTweenTag Tag;
    ETweenState State = ETweenState::Pending;

    TArray<FSequenceAction> Actions;
    float Cursor = 0.f;
    float TotalDuration = 0.f;

    ETweenTimeMode TimeMode = ETweenTimeMode::GameTime;

    ETweenLoopMode LoopMode = ETweenLoopMode::None;
    int32 LoopCount = 0;
    int32 CurrentLoop = 0;

    TWeakObjectPtr<UObject> BoundTarget;
    bool bBoundTargetExplicit = false;

    TWeakObjectPtr<UTweenSubsystem> OwningSubsystem;

    TFunction<void()> OnCompleteCallback;
    TFunction<void()> OnKillCallback;

    void Tick(float DeltaTime);
    void Play();
    void Pause();
    void Resume();
    void Restart();
    void Kill();

    bool IsComplete() const { return State == ETweenState::Completed; }
    bool IsKilled() const { return State == ETweenState::Killed; }

    void AddTweenAction(TSharedPtr<FTweenInstanceBase> Instance, bool bJoin);
    void AddSequenceAction(TSharedPtr<FTweenSequence> SubSeq, bool bJoin);
    void AddIntervalAction(float InDuration);
    void AddCallbackAction(TFunction<void()> Callback);

private:
    float InsertCursor = 0.f;
    float LastAppendStart = 0.f;
    float LastAppendDuration = 0.f;

    void HandleCompletion();
    void ResetForLoop();
    bool AllActionsComplete() const;
};

/** Fluent handle for building and controlling a sequence. Safe no-op when expired. */
struct UTWEEN_API FTweenSequenceHandle
{
    FTweenSequenceHandle() = default;
    explicit FTweenSequenceHandle(TSharedPtr<FTweenSequence> InSequence);

    FTweenSequenceHandle& Append(FTweenHandle Tween);
    FTweenSequenceHandle& Append(FTweenSequenceHandle SubSequence);
    FTweenSequenceHandle& Join(FTweenHandle Tween);
    FTweenSequenceHandle& Join(FTweenSequenceHandle SubSequence);
    FTweenSequenceHandle& AppendInterval(float InDuration);
    FTweenSequenceHandle& AppendCallback(TFunction<void()> Callback);

    FTweenSequenceHandle& SetLoops(int32 Count, ETweenLoopMode Mode = ETweenLoopMode::Restart);
    FTweenSequenceHandle& SetTimeMode(ETweenTimeMode Mode);
    FTweenSequenceHandle& SetTag(FTweenTag InTag);
    FTweenSequenceHandle& SetBoundTarget(UObject* Target);
    FTweenSequenceHandle& OnComplete(TFunction<void()> Callback);
    FTweenSequenceHandle& OnKill(TFunction<void()> Callback);

    FTweenSequenceHandle& Play();
    void Pause();
    void Resume();
    void Restart();
    void Kill();

    bool IsValid() const;
    bool IsPlaying() const;
    FTweenId GetId() const { return Id; }
    TSharedPtr<FTweenSequence> GetInstance() const { return SequencePtr.Pin(); }

private:
    TWeakPtr<FTweenSequence> SequencePtr;
    FTweenId Id;

    FTweenSequence* Pin() const;
    void AttachTweenHandle(FTweenHandle Tween, bool bJoin);
    void AttachSequenceHandle(FTweenSequenceHandle SubSequence, bool bJoin);
};
