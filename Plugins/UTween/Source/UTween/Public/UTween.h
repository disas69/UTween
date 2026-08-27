#pragma once

#include "CoreMinimal.h"
#include "TweenHandle.h"
#include "TweenSequence.h"
#include "TweenInstance.h"
#include "UTweenSubsystem.h"
#include "UTweenModule.h"
#include "Templates/Function.h"
#include "Templates/UnrealTypeTraits.h"
#include "UObject/WeakObjectPtr.h"
#include <type_traits>

class UWidget;
class UCanvasPanelSlot;
class AActor;
class USceneComponent;
class UMaterialInstanceDynamic;

/**
 * Static factory for creating tweens. Each method creates, registers, and
 * returns a fluent FTweenHandle for configuration and control.
 */
class UTWEEN_API UTween
{
public:
    UTween() = delete;

    static FTweenHandle Value(
        float From, float To, float Duration,
        TFunction<void(float)> OnUpdate);
    static FTweenHandle Value(
        const UObject* WorldContext,
        float From, float To, float Duration,
        TFunction<void(float)> OnUpdate);

    static FTweenHandle Value(
        const FVector2D& From, const FVector2D& To, float Duration,
        TFunction<void(const FVector2D&)> OnUpdate);
    static FTweenHandle Value(
        const UObject* WorldContext,
        const FVector2D& From, const FVector2D& To, float Duration,
        TFunction<void(const FVector2D&)> OnUpdate);

    static FTweenHandle Value(
        const FVector& From, const FVector& To, float Duration,
        TFunction<void(const FVector&)> OnUpdate);
    static FTweenHandle Value(
        const UObject* WorldContext,
        const FVector& From, const FVector& To, float Duration,
        TFunction<void(const FVector&)> OnUpdate);

    static FTweenHandle Value(
        const FRotator& From, const FRotator& To, float Duration,
        TFunction<void(const FRotator&)> OnUpdate);
    static FTweenHandle Value(
        const UObject* WorldContext,
        const FRotator& From, const FRotator& To, float Duration,
        TFunction<void(const FRotator&)> OnUpdate);

    static FTweenHandle Value(
        const FLinearColor& From, const FLinearColor& To, float Duration,
        TFunction<void(const FLinearColor&)> OnUpdate);
    static FTweenHandle Value(
        const UObject* WorldContext,
        const FLinearColor& From, const FLinearColor& To, float Duration,
        TFunction<void(const FLinearColor&)> OnUpdate);

    // Widgets
    static FTweenHandle WidgetPosition(UWidget* Target, const FVector2D& To, float Duration);
    static FTweenHandle CanvasPosition(UWidget* Target, const FVector2D& To, float Duration);
    static FTweenHandle CanvasPosition(UCanvasPanelSlot* Target, const FVector2D& To, float Duration);
    static FTweenHandle CanvasSize(UWidget* Target, const FVector2D& To, float Duration);
    static FTweenHandle CanvasSize(UCanvasPanelSlot* Target, const FVector2D& To, float Duration);
    static FTweenHandle WidgetScale(UWidget* Target, const FVector2D& To, float Duration);
    static FTweenHandle WidgetRotation(UWidget* Target, float To, float Duration);
    static FTweenHandle WidgetOpacity(UWidget* Target, float To, float Duration);
    static FTweenHandle WidgetColor(UWidget* Target, const FLinearColor& To, float Duration);

    // Actors
    static FTweenHandle ActorLocation(AActor* Target, const FVector& To, float Duration);
    static FTweenHandle ActorRotation(AActor* Target, const FRotator& To, float Duration);
    static FTweenHandle ActorScale(AActor* Target, const FVector& To, float Duration);

    // Components
    static FTweenHandle ComponentRelativeLocation(USceneComponent* Target, const FVector& To, float Duration);
    static FTweenHandle ComponentRelativeRotation(USceneComponent* Target, const FRotator& To, float Duration);
    static FTweenHandle ComponentRelativeScale(USceneComponent* Target, const FVector& To, float Duration);

    // Materials
    static FTweenHandle MaterialScalar(UMaterialInstanceDynamic* Target, FName ParamName, float To, float Duration);
    static FTweenHandle MaterialVector(UMaterialInstanceDynamic* Target, FName ParamName, const FLinearColor& To, float Duration);

    // Unified dispatchers
    static FTweenHandle Position(AActor* Target, const FVector& From, const FVector& To, float Duration);
    static FTweenHandle Position(USceneComponent* Target, const FVector& From, const FVector& To, float Duration);
    static FTweenHandle Position(UWidget* Target, const FVector2D& From, const FVector2D& To, float Duration);

    static FTweenHandle Scale(AActor* Target, const FVector& From, const FVector& To, float Duration);
    static FTweenHandle Scale(USceneComponent* Target, const FVector& From, const FVector& To, float Duration);
    static FTweenHandle Scale(UWidget* Target, const FVector2D& From, const FVector2D& To, float Duration);

    static FTweenHandle Rotation(AActor* Target, const FRotator& From, const FRotator& To, float Duration);
    static FTweenHandle Rotation(USceneComponent* Target, const FRotator& From, const FRotator& To, float Duration);
    static FTweenHandle Rotation(UWidget* Target, float From, float To, float Duration);

    static FTweenHandle Opacity(UWidget* Target, float From, float To, float Duration);

    static FTweenHandle Color(UWidget* Target, const FLinearColor& From, const FLinearColor& To, float Duration);

    /** Tween a value applied via a member function pointer, lambda, or free callable. */
    template <typename TObj, typename TValue, typename TCallable>
    static FTweenHandle ValueMember(TObj* Obj, TValue From, TValue To, float Duration, TCallable&& Callback);

    static FTweenSequenceHandle Sequence(const UObject* WorldContext = nullptr);

    /** Resolve the active subsystem with world context awareness and GEngine fallback. */
    static UTweenSubsystem* GetSubsystem(const UObject* WorldContext = nullptr);

private:
    template <typename T>
    static FTweenHandle CreateValueTween(
        const UObject* WorldContext,
        const T& From, const T& To, float Duration,
        TFunction<void(const T&)> OnUpdate,
        const TCHAR* MethodName);

    template <typename TValue>
    static FTweenHandle CreateTargetTween(
        UObject* Target,
        const TValue& To,
        float Duration,
        TFunction<TValue()> Getter,
        TFunction<void(const TValue&)> Apply,
        TFunction<TValue(const TValue&, const TValue&, float)> Lerp,
        const TCHAR* MethodName);
};

template <typename TObj, typename TValue, typename TCallable>
FTweenHandle UTween::ValueMember(TObj* Obj, TValue From, TValue To, float Duration, TCallable&& Callback)
{
    if (!Obj)
    {
        return FTweenHandle();
    }

    constexpr bool bIsMember = std::is_member_function_pointer_v<std::decay_t<TCallable>>;
    constexpr bool bIsUObj = TIsDerivedFrom<TObj, UObject>::Value;

    const UObject* WorldContext = nullptr;
    if constexpr (bIsUObj)
    {
        WorldContext = Obj;
    }

    UTweenSubsystem* Subsystem = GetSubsystem(WorldContext);
    if (!Subsystem)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::ValueMember called with no active subsystem; returning invalid handle."));
        return FTweenHandle();
    }

    TSharedPtr<TTweenInstance<TValue>> Instance = MakeShared<TTweenInstance<TValue>>();
    Instance->StartValue = From;
    Instance->EndValue = To;
    Instance->Duration = FMath::Max(0.f, Duration);
    Instance->bUseFrom = true;

    if constexpr (std::is_same_v<TValue, FRotator>)
    {
        Instance->LerpFunc = &UTweenLerp::LerpRotator;
    }
    else
    {
        Instance->LerpFunc = [](const TValue& A, const TValue& B, float Alpha)
        {
            return FMath::Lerp(A, B, Alpha);
        };
    }

    if constexpr (bIsUObj)
    {
        TWeakObjectPtr<TObj> Weak(Obj);
        Instance->BoundTarget = Obj;
        Instance->bHasTarget = (Obj != nullptr);

        if constexpr (bIsMember)
        {
            auto Member = Callback;
            Instance->ApplyFunc = [Weak, Member](const TValue& V)
            {
                if (TObj* O = Weak.Get())
                {
                    (O->*Member)(V);
                }
            };
        }
        else
        {
            Instance->ApplyFunc = [Weak, Fn = std::forward<TCallable>(Callback)](const TValue& V)
            {
                if (Weak.IsValid())
                {
                    Fn(V);
                }
            };
        }
    }
    else
    {
        if constexpr (bIsMember)
        {
            auto Member = Callback;
            Instance->ApplyFunc = [Obj, Member](const TValue& V)
            {
                (Obj->*Member)(V);
            };
        }
        else
        {
            Instance->ApplyFunc = [Obj, Fn = std::forward<TCallable>(Callback)](const TValue& V)
            {
                Fn(V);
            };
        }
    }

    Subsystem->RegisterTween(Instance);
    return FTweenHandle(Instance);
}
