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
    bool bHasTarget = false;

    uint64 ValueTypeHash = 0;

    void Tick(float DeltaTime);
    void Kill();
    void Pause();
    void Resume();
    void Complete();
    void Play();
    void Restart();

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
    /**
     * Rotator interpolation supporting both shortest-path orientation and multi-turn spins.
     * When any component difference indicates an intentional multi-turn or 360-degree spin
     * (|delta| >= 360 deg), or when start and end are identical in quaternion space (which
     * would otherwise cause Slerp to evaluate to a degenerate 0-degree motion), component-wise
     * Euler interpolation is used. Otherwise, spherical linear interpolation (quaternion slerp)
     * is used for shortest-path orientation.
     */
    FORCEINLINE FRotator LerpRotator(const FRotator& A, const FRotator& B, float Alpha)
    {
        const float DeltaPitch = FMath::Abs(B.Pitch - A.Pitch);
        const float DeltaYaw   = FMath::Abs(B.Yaw - A.Yaw);
        const float DeltaRoll  = FMath::Abs(B.Roll - A.Roll);

        if (DeltaPitch >= 360.f || DeltaYaw >= 360.f || DeltaRoll >= 360.f)
        {
            return FRotator(
                FMath::Lerp(A.Pitch, B.Pitch, Alpha),
                FMath::Lerp(A.Yaw, B.Yaw, Alpha),
                FMath::Lerp(A.Roll, B.Roll, Alpha)
            );
        }

        const FQuat QA = A.Quaternion();
        const FQuat QB = B.Quaternion();

        if (QA.Equals(QB, 1e-4f) || QA.Equals(-QB, 1e-4f))
        {
            return FRotator(
                FMath::Lerp(A.Pitch, B.Pitch, Alpha),
                FMath::Lerp(A.Yaw, B.Yaw, Alpha),
                FMath::Lerp(A.Roll, B.Roll, Alpha)
            );
        }

        return FQuat::Slerp(QA, QB, Alpha).Rotator();
    }
}

