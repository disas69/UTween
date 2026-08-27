#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TweenTypes.h"
#include "TweenBlueprintTypes.h"
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
};
