#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UTweenFluentExample.generated.h"

class UUserWidget;
class UMaterialInstanceDynamic;
class USceneComponent;

/**
 * Mirror of AUTweenMacrosExample written against the fluent UTween API
 * directly (no macros). Same demo cases.
 */
UCLASS(Blueprintable, BlueprintType)
class UTWEENPROJECT_API AUTweenFluentExample : public AActor
{
    GENERATED_BODY()

public:
    AUTweenFluentExample();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UTween|Demo")
    TObjectPtr<UUserWidget> DemoWidget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UTween|Demo")
    TObjectPtr<UMaterialInstanceDynamic> DemoMaterial = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UTween|Demo")
    float CurrentAlpha = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UTween|Demo")
    FVector CurrentOffset = FVector::ZeroVector;

    UFUNCTION(BlueprintCallable, Category = "UTween|Demo")
    void RunAllDemos();

protected:
    virtual void BeginPlay() override;

    void OnAlphaUpdate(float NewAlpha);
    void OnOffsetUpdate(const FVector& NewOffset);

    void DemoGenericValue();
    void DemoTargetSpecific();
    void DemoSequence();
    void DemoParallel();
    void DemoNested();

private:
    UPROPERTY()
    TObjectPtr<USceneComponent> Root = nullptr;
};
