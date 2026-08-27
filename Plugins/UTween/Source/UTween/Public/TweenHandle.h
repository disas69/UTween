#pragma once

#include "CoreMinimal.h"
#include "TweenTypes.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"

class FTweenInstanceBase;

/**
 * Lightweight copyable handle to a running tween. Holds a weak pointer
 * to the instance; expired tweens become safe no-ops.
 */
struct UTWEEN_API FTweenHandle
{
    FTweenHandle() = default;
    explicit FTweenHandle(TSharedPtr<FTweenInstanceBase> InInstance);

    FTweenHandle& SetEase(ETweenEase InEase);
    FTweenHandle& SetDelay(float InDelay);
    FTweenHandle& SetLoops(int32 InLoopCount, ETweenLoopMode InMode = ETweenLoopMode::Restart);
    FTweenHandle& SetAutoKill(bool bInAutoKill);
    FTweenHandle& SetTimeMode(ETweenTimeMode InMode);
    FTweenHandle& SetTag(FTweenTag InTag);
    FTweenHandle& SetRelative(bool bInRelative = true);

    FTweenHandle& From(float InValue);
    FTweenHandle& From(const FVector2D& InValue);
    FTweenHandle& From(const FVector& InValue);
    FTweenHandle& From(const FRotator& InValue);
    FTweenHandle& From(const FLinearColor& InValue);

    FTweenHandle& OnStart(TFunction<void()> Callback);
    FTweenHandle& OnUpdate(TFunction<void()> Callback);
    FTweenHandle& OnComplete(TFunction<void()> Callback);
    FTweenHandle& OnKill(TFunction<void()> Callback);

    void Play();
    void Restart();
    void Pause();
    void Resume();
    void Complete();
    void Kill();

    bool IsValid() const;
    bool IsPlaying() const;
    ETweenState GetState() const;
    FTweenId GetId() const { return Id; }
    TSharedPtr<FTweenInstanceBase> GetInstance() const { return InstancePtr.Pin(); }

private:
    TWeakPtr<FTweenInstanceBase> InstancePtr;
    FTweenId Id;

    FTweenInstanceBase* Pin() const;
};
