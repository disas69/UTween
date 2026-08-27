#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TweenTypes.h"
#include "TweenBlueprintTypes.h"
#include "Engine/LatentActionManager.h"
#include "UTweenBlueprintLibrary.generated.h"

class UWidget;
class UMaterialInstanceDynamic;
class USceneComponent;
class AActor;

/** Blueprint facade over the UTween C++ API. */
UCLASS()
class UTWEEN_API UTweenBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UTween|Widget",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenWidgetPosition(
        UObject* WorldContextObject,
        UWidget* Target,
        FVector2D To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Widget",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenWidgetScale(
        UObject* WorldContextObject,
        UWidget* Target,
        FVector2D To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Widget",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenWidgetRotation(
        UObject* WorldContextObject,
        UWidget* Target,
        float To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Widget",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenWidgetOpacity(
        UObject* WorldContextObject,
        UWidget* Target,
        float To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Widget",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenWidgetColor(
        UObject* WorldContextObject,
        UWidget* Target,
        FLinearColor To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Widget",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenCanvasPosition(
        UObject* WorldContextObject,
        UWidget* Target,
        FVector2D To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Widget",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenCanvasSize(
        UObject* WorldContextObject,
        UWidget* Target,
        FVector2D To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Actor",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenActorLocation(
        UObject* WorldContextObject,
        AActor* Target,
        FVector To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Actor",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenActorRotation(
        UObject* WorldContextObject,
        AActor* Target,
        FRotator To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Actor",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenActorScale(
        UObject* WorldContextObject,
        AActor* Target,
        FVector To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Component",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenComponentRelativeLocation(
        UObject* WorldContextObject,
        USceneComponent* Target,
        FVector To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Component",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenComponentRelativeRotation(
        UObject* WorldContextObject,
        USceneComponent* Target,
        FRotator To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Component",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenComponentRelativeScale(
        UObject* WorldContextObject,
        USceneComponent* Target,
        FVector To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Material",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenMaterialScalarParameter(
        UObject* WorldContextObject,
        UMaterialInstanceDynamic* Target,
        FName ParameterName,
        float To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Material",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenMaterialVectorParameter(
        UObject* WorldContextObject,
        UMaterialInstanceDynamic* Target,
        FName ParameterName,
        FLinearColor To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Value",
        meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "OnUpdate"))
    static FBPTweenHandle TweenFloat(
        UObject* WorldContextObject,
        float From,
        float To,
        float Duration,
        const FOnTweenFloatUpdate& OnUpdate,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Value",
        meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "OnUpdate"))
    static FBPTweenHandle TweenVector(
        UObject* WorldContextObject,
        FVector From,
        FVector To,
        float Duration,
        const FOnTweenVectorUpdate& OnUpdate,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Value",
        meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "OnUpdate"))
    static FBPTweenHandle TweenColor(
        UObject* WorldContextObject,
        FLinearColor From,
        FLinearColor To,
        float Duration,
        const FOnTweenColorUpdate& OnUpdate,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void PauseTween(UObject* WorldContextObject, FBPTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void ResumeTween(UObject* WorldContextObject, FBPTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void KillTween(UObject* WorldContextObject, FBPTweenHandle Handle);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static bool IsTweenValid(UObject* WorldContextObject, FBPTweenHandle Handle);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static bool IsTweenPlaying(UObject* WorldContextObject, FBPTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenSequenceHandle CreateTweenSequence(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenSequenceHandle AppendToSequence(
        UObject* WorldContextObject,
        FBPTweenSequenceHandle Sequence,
        FBPTweenHandle Tween);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenSequenceHandle JoinToSequence(
        UObject* WorldContextObject,
        FBPTweenSequenceHandle Sequence,
        FBPTweenHandle Tween);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenSequenceHandle AppendIntervalToSequence(
        UObject* WorldContextObject,
        FBPTweenSequenceHandle Sequence,
        float Duration);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static void PlaySequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static void PauseSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static void KillSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static bool IsSequenceValid(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static bool IsSequencePlaying(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void PauseAllTweens(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void ResumeAllTweens(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void KillAllTweens(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void PauseTweensByTag(UObject* WorldContextObject, FName Tag);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void ResumeTweensByTag(UObject* WorldContextObject, FName Tag);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void KillTweensByTag(UObject* WorldContextObject, FName Tag);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UTween|Debug",
        meta = (WorldContext = "WorldContextObject"))
    static int32 GetActiveTweenCount(UObject* WorldContextObject);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UTween|Debug",
        meta = (WorldContext = "WorldContextObject"))
    static int32 GetActiveSequenceCount(UObject* WorldContextObject);

    // Handle configuration & fluent setters
    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle SetDelay(UObject* WorldContextObject, FBPTweenHandle Handle, float Delay);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle SetLoops(UObject* WorldContextObject, FBPTweenHandle Handle, int32 Count, ETweenLoopMode Mode = ETweenLoopMode::Restart);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle SetEase(UObject* WorldContextObject, FBPTweenHandle Handle, ETweenEase Ease);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle SetRelative(UObject* WorldContextObject, FBPTweenHandle Handle, bool bRelative = true);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle SetTimeMode(UObject* WorldContextObject, FBPTweenHandle Handle, ETweenTimeMode Mode);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle SetTag(UObject* WorldContextObject, FBPTweenHandle Handle, FName Tag);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle FromFloat(UObject* WorldContextObject, FBPTweenHandle Handle, float FromValue);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle FromVector2D(UObject* WorldContextObject, FBPTweenHandle Handle, FVector2D FromValue);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle FromVector(UObject* WorldContextObject, FBPTweenHandle Handle, FVector FromValue);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle FromRotator(UObject* WorldContextObject, FBPTweenHandle Handle, FRotator FromValue);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle FromColor(UObject* WorldContextObject, FBPTweenHandle Handle, FLinearColor FromValue);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Callback"))
    static FBPTweenHandle OnTweenComplete(UObject* WorldContextObject, FBPTweenHandle Handle, const FOnTweenDynamicCallback& Callback);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Callback"))
    static FBPTweenHandle OnTweenStart(UObject* WorldContextObject, FBPTweenHandle Handle, const FOnTweenDynamicCallback& Callback);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Callback"))
    static FBPTweenHandle OnTweenKill(UObject* WorldContextObject, FBPTweenHandle Handle, const FOnTweenDynamicCallback& Callback);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void PlayTween(UObject* WorldContextObject, FBPTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void RestartTween(UObject* WorldContextObject, FBPTweenHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "UTween|Control",
        meta = (WorldContext = "WorldContextObject"))
    static void CompleteTween(UObject* WorldContextObject, FBPTweenHandle Handle);

    // Sequence configuration
    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenSequenceHandle SetSequenceLoops(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, int32 Count, ETweenLoopMode Mode = ETweenLoopMode::Restart);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenSequenceHandle SetSequenceTimeMode(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, ETweenTimeMode Mode);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenSequenceHandle SetSequenceTag(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, FName Tag);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Callback"))
    static FBPTweenSequenceHandle OnSequenceComplete(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, const FOnTweenDynamicCallback& Callback);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Callback"))
    static FBPTweenSequenceHandle OnSequenceKill(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, const FOnTweenDynamicCallback& Callback);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenSequenceHandle AppendSubSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, FBPTweenSequenceHandle SubSequence);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenSequenceHandle JoinSubSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, FBPTweenSequenceHandle SubSequence);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static void RestartSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence);

    UFUNCTION(BlueprintCallable, Category = "UTween|Sequence",
        meta = (WorldContext = "WorldContextObject"))
    static void ResumeSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence);

    // Latent Actions
    UFUNCTION(BlueprintCallable, Category = "UTween|Latent",
        meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
    static void TweenActorLocationLatent(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        AActor* Target,
        FVector To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Latent",
        meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
    static void TweenActorRotationLatent(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        AActor* Target,
        FRotator To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Latent",
        meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
    static void TweenActorScaleLatent(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        AActor* Target,
        FVector To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    UFUNCTION(BlueprintCallable, Category = "UTween|Latent",
        meta = (Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
    static void TweenWidgetPositionLatent(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        UWidget* Target,
        FVector2D To,
        float Duration,
        ETweenEase Ease = ETweenEase::OutQuad);

    // Juice (Punch & Shake)
    UFUNCTION(BlueprintCallable, Category = "UTween|Juice",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenActorPunchPosition(
        UObject* WorldContextObject,
        AActor* Target,
        FVector Punch,
        float Duration = 0.5f,
        int32 Vibrato = 10,
        float Elasticity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "UTween|Juice",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenActorPunchScale(
        UObject* WorldContextObject,
        AActor* Target,
        FVector Punch,
        float Duration = 0.5f,
        int32 Vibrato = 10,
        float Elasticity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "UTween|Juice",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenActorShakePosition(
        UObject* WorldContextObject,
        AActor* Target,
        float Strength,
        float Duration = 0.5f,
        int32 Vibrato = 10,
        float Randomness = 90.0f);

    UFUNCTION(BlueprintCallable, Category = "UTween|Juice",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenWidgetPunchPosition(
        UObject* WorldContextObject,
        UWidget* Target,
        FVector2D Punch,
        float Duration = 0.5f,
        int32 Vibrato = 10,
        float Elasticity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "UTween|Juice",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenWidgetPunchScale(
        UObject* WorldContextObject,
        UWidget* Target,
        FVector2D Punch,
        float Duration = 0.5f,
        int32 Vibrato = 10,
        float Elasticity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "UTween|Juice",
        meta = (WorldContext = "WorldContextObject"))
    static FBPTweenHandle TweenWidgetShakePosition(
        UObject* WorldContextObject,
        UWidget* Target,
        float Strength,
        float Duration = 0.5f,
        int32 Vibrato = 10);
};
