#pragma once

#include "CoreMinimal.h"
#include "TweenTypes.h"
#include "TweenEasing.h"
#include "UObject/WeakObjectPtr.h"
#include "Templates/Function.h"
#include "Templates/SharedPointer.h"
#include <type_traits>

template <typename T>
using TTweenApplyArg = std::conditional_t<std::is_arithmetic_v<T>, T, const T&>;

template <typename T>
struct TTweenValueTypeTraits
{
    static constexpr ETweenValueType ValueType = ETweenValueType::Custom;
};

template <> struct TTweenValueTypeTraits<float> { static constexpr ETweenValueType ValueType = ETweenValueType::Float; };
template <> struct TTweenValueTypeTraits<FVector2D> { static constexpr ETweenValueType ValueType = ETweenValueType::Vector2D; };
template <> struct TTweenValueTypeTraits<FVector> { static constexpr ETweenValueType ValueType = ETweenValueType::Vector; };
template <> struct TTweenValueTypeTraits<FRotator> { static constexpr ETweenValueType ValueType = ETweenValueType::Rotator; };
template <> struct TTweenValueTypeTraits<FLinearColor> { static constexpr ETweenValueType ValueType = ETweenValueType::LinearColor; };

class FTweenSequence;
class UTweenSubsystem;

/** Abstract base for all tween instances, agnostic of the value type being tweened. */

class UTWEEN_API FTweenInstanceBase : public TSharedFromThis<FTweenInstanceBase>
{
public:
    virtual ~FTweenInstanceBase() = default;

    friend class FTweenSequence;
    friend class UTweenSubsystem;

    FTweenId Id;
    FTweenTag Tag;
    FName DebugName;
    int32 ActiveIndex = INDEX_NONE;

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
    bool bRestoreStartOnKill = false;
    ETweenTimeMode TimeMode = ETweenTimeMode::GameTime;

    ETweenState State = ETweenState::Pending;

    TFunction<void()> OnStartCallback;
    TFunction<void()> OnUpdateCallback;
    TFunction<void()> OnCompleteCallback;
    TFunction<void()> OnKillCallback;

    TWeakObjectPtr<UObject> BoundTarget;
    bool bHasTarget = false;
    TWeakObjectPtr<UTweenSubsystem> OwningSubsystem;

    ETweenValueType ValueType = ETweenValueType::Unknown;

    void Tick(float DeltaTime);
    void Kill();
    void Pause();
    void Resume();
    void Complete();
    void Play();
    void Restart();
    virtual void ResetForPool();

    bool IsComplete() const { return State == ETweenState::Completed; }
    bool IsKilled() const { return State == ETweenState::Killed; }
    bool IsTargetValid() const;

    template <typename T>
    bool IsType() const
    {
        return ValueType == TTweenValueTypeTraits<T>::ValueType;
    }

protected:
    virtual void EvaluateAndApply(float Alpha) = 0;
    virtual void CaptureStartValue() = 0;
    virtual void ApplyRelativeOffset() {}
    virtual void SwapStartAndEnd() {}
    virtual void OnValuesPrepared() {}

    void HandleLoopOrComplete();

    bool bStartFired = false;
    bool bValuesInitialized = false;
};

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
    FORCEINLINE bool ShouldUseEulerRotatorLerp(const FRotator& A, const FRotator& B, FQuat& OutQA, FQuat& OutQB)
    {
        const float DeltaPitch = FMath::Abs(B.Pitch - A.Pitch);
        const float DeltaYaw   = FMath::Abs(B.Yaw - A.Yaw);
        const float DeltaRoll  = FMath::Abs(B.Roll - A.Roll);

        if (DeltaPitch >= 360.f || DeltaYaw >= 360.f || DeltaRoll >= 360.f)
        {
            return true;
        }

        OutQA = A.Quaternion();
        OutQB = B.Quaternion();

        return OutQA.Equals(OutQB, 1e-4f) || OutQA.Equals(-OutQB, 1e-4f);
    }

    FORCEINLINE FRotator LerpRotatorEuler(const FRotator& A, const FRotator& B, float Alpha)
    {
        return FRotator(
            FMath::Lerp(A.Pitch, B.Pitch, Alpha),
            FMath::Lerp(A.Yaw, B.Yaw, Alpha),
            FMath::Lerp(A.Roll, B.Roll, Alpha)
        );
    }

    FORCEINLINE FRotator LerpRotator(const FRotator& A, const FRotator& B, float Alpha)
    {
        FQuat QA, QB;
        if (ShouldUseEulerRotatorLerp(A, B, QA, QB))
        {
            return LerpRotatorEuler(A, B, Alpha);
        }

        return FQuat::Slerp(QA, QB, Alpha).Rotator();
    }
}

namespace UTweenDefaults
{
    template <typename T>
    FORCEINLINE T DefaultLerp(const T& A, const T& B, float Alpha)
    {
        return FMath::Lerp(A, B, Alpha);
    }

    template <>
    FORCEINLINE FRotator DefaultLerp<FRotator>(const FRotator& A, const FRotator& B, float Alpha)
    {
        return UTweenLerp::LerpRotator(A, B, Alpha);
    }

    template <typename T>
    TFunction<T(const T&, const T&, float)> MakeLerp()
    {
        return [](const T& A, const T& B, float Alpha) -> T
        {
            return DefaultLerp<T>(A, B, Alpha);
        };
    }
}

template <typename T>
struct TTweenEndpointsCache
{
    FORCEINLINE void Prepare(const T&, const T&) {}
    FORCEINLINE void SwapEndpoints() {}
    FORCEINLINE void Reset() {}
    FORCEINLINE T EvaluateDefault(const T& Start, const T& End, float Alpha) const
    {
        return UTweenDefaults::DefaultLerp<T>(Start, End, Alpha);
    }
};

template <>
struct TTweenEndpointsCache<FRotator>
{
    FQuat StartQuat{ForceInit};
    FQuat EndQuat{ForceInit};
    bool bUseEuler = true;

    FORCEINLINE void Prepare(const FRotator& Start, const FRotator& End)
    {
        bUseEuler = UTweenLerp::ShouldUseEulerRotatorLerp(Start, End, StartQuat, EndQuat);
    }

    FORCEINLINE void SwapEndpoints()
    {
        Swap(StartQuat, EndQuat);
    }

    FORCEINLINE void Reset()
    {
        bUseEuler = true;
    }

    FORCEINLINE FRotator EvaluateDefault(const FRotator& Start, const FRotator& End, float Alpha) const
    {
        if (bUseEuler)
        {
            return UTweenLerp::LerpRotatorEuler(Start, End, Alpha);
        }
        return FQuat::Slerp(StartQuat, EndQuat, Alpha).Rotator();
    }
};

/** Typed tween instance carrying start/end values, lerp, apply, and getter. */
template <typename T>
class TTweenInstance : public FTweenInstanceBase
{
public:
    T StartValue{};
    T EndValue{};
    T CurrentValue{};
    TTweenEndpointsCache<T> EndpointsCache;

    TFunction<T(const T&, const T&, float)> LerpFunc;
    TFunction<void(TTweenApplyArg<T>)> ApplyFunc;
    TFunction<T()> GetterFunc;

    TTweenInstance()
    {
        ValueType = TTweenValueTypeTraits<T>::ValueType;
    }

    virtual void ResetForPool() override
    {
        FTweenInstanceBase::ResetForPool();
        StartValue = T{};
        EndValue = T{};
        CurrentValue = T{};
        EndpointsCache.Reset();
        LerpFunc = nullptr;
        ApplyFunc = nullptr;
        GetterFunc = nullptr;
    }

protected:
    virtual void EvaluateAndApply(float Alpha) override
    {
        if (UNLIKELY(LerpFunc != nullptr))
        {
            CurrentValue = LerpFunc(StartValue, EndValue, Alpha);
        }
        else
        {
            CurrentValue = EndpointsCache.EvaluateDefault(StartValue, EndValue, Alpha);
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
        EndpointsCache.SwapEndpoints();
    }

    virtual void OnValuesPrepared() override
    {
        EndpointsCache.Prepare(StartValue, EndValue);
    }
};
