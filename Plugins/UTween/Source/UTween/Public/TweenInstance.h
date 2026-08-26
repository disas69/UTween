#pragma once

#include "CoreMinimal.h"
#include "TweenTypes.h"
#include "TweenEasing.h"
#include "UObject/WeakObjectPtr.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"

/**
 * RTTI-free per-type identity. Uses the address of a per-type static
 * sentinel as a stable unique id across modules that disable RTTI.
 */
template <typename T>
struct TTweenValueTypeTag
{
    static const uint8 Sentinel;
};

template <typename T>
const uint8 TTweenValueTypeTag<T>::Sentinel = 0;

template <typename T>
FORCEINLINE uint64 GetTweenValueTypeHash()
{
    return reinterpret_cast<uint64>(&TTweenValueTypeTag<T>::Sentinel);
}

class FTweenSequence;

/** Abstract base for all tween instances, agnostic of the value type being tweened. */

class UTWEEN_API FTweenInstanceBase : public TSharedFromThis<FTweenInstanceBase>
{
public:
    virtual ~FTweenInstanceBase() = default;

    friend class FTweenSequence;

    FTweenId Id;
    FTweenTag Tag;
    FName DebugName;

    float Duration = 0.f;
    float Elapsed = 0.f;
    float Delay = 0.f;
    float DelayElapsed = 0.f;

    ETweenEase Ease = ETweenEase::OutQuad;
    ETweenLoopMode LoopMode = ETweenLoopMode::None;
    int32 LoopCount = 0;
    int32 CurrentLoop = 0;
    bool bAutoKill = true;
    bool bUseFrom = false;
    bool bIsRelative = false;
    ETweenTimeMode TimeMode = ETweenTimeMode::GameTime;

    ETweenState State = ETweenState::Pending;

    TFunction<void()> OnStartCallback;
    TFunction<void()> OnUpdateCallback;
    TFunction<void()> OnCompleteCallback;
    TFunction<void()> OnKillCallback;

    TWeakObjectPtr<UObject> BoundTarget;

    uint64 ValueTypeHash = 0;

    void Tick(float DeltaTime);
    void Kill();
    void Pause();
    void Resume();
    void Complete();

    bool IsComplete() const { return State == ETweenState::Completed; }
    bool IsKilled() const { return State == ETweenState::Killed; }
    bool IsTargetValid() const;

    template <typename T>
    bool IsType() const
    {
        return ValueTypeHash == GetTweenValueTypeHash<T>();
    }

protected:
    virtual void EvaluateAndApply(float Alpha) = 0;
    virtual void CaptureStartValue() = 0;
    virtual void ApplyRelativeOffset() {}
    virtual void SwapStartAndEnd() {}

    void HandleLoopOrComplete();

    bool bStartFired = false;
};

/** Typed tween instance carrying start/end values, lerp, apply, and getter. */
template <typename T>
class TTweenInstance : public FTweenInstanceBase
{
public:
    T StartValue{};
    T EndValue{};
    T CurrentValue{};

    TFunction<T(const T&, const T&, float)> LerpFunc;
    TFunction<void(const T&)> ApplyFunc;
    TFunction<T()> GetterFunc;

    TTweenInstance()
    {
        ValueTypeHash = GetTweenValueTypeHash<T>();
    }

protected:
    virtual void EvaluateAndApply(float Alpha) override
    {
        if (LerpFunc)
        {
            CurrentValue = LerpFunc(StartValue, EndValue, Alpha);
        }
        if (ApplyFunc)
        {
            ApplyFunc(CurrentValue);
        }
    }

    virtual void CaptureStartValue() override
    {
        if (GetterFunc)
        {
            StartValue = GetterFunc();
        }
    }

    virtual void ApplyRelativeOffset() override
    {
        EndValue = StartValue + EndValue;
    }

    virtual void SwapStartAndEnd() override
    {
        Swap(StartValue, EndValue);
    }
};

namespace UTweenDefaults
{
    template <typename T>
    TFunction<T(const T&, const T&, float)> MakeLerp()
    {
        return [](const T& A, const T& B, float Alpha) -> T
        {
            return FMath::Lerp(A, B, Alpha);
        };
    }
}

namespace UTweenLerp
{
    /** Shortest-path rotator interpolation via quaternion slerp. */
    FORCEINLINE FRotator LerpRotator(const FRotator& A, const FRotator& B, float Alpha)
    {
        const FQuat QA = A.Quaternion();
        const FQuat QB = B.Quaternion();
        return FQuat::Slerp(QA, QB, Alpha).Rotator();
    }
}
