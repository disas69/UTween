#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TweenHandle.h"
#include "JuiceDummyActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDummyHitSignature, float, Damage, const FVector&, HitWorldLocation, bool, bIsCritical, int32, ComboCount);

UCLASS(Blueprintable, BlueprintType)
class UTWEENPROJECT_API AJuiceDummyActor : public AActor
{
    GENERATED_BODY()

public:
    AJuiceDummyActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BasePedestal;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> WobblePivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> StemPole;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> TorsoMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HeadMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> TargetDisc;

    UPROPERTY(BlueprintAssignable, Category = "JuiceLab|Dummy")
    FOnDummyHitSignature OnDummyHit;

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Dummy")
    void TakeHit(float Damage, FVector HitDirection, bool bIsCritical = false);

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Dummy")
    void ResetCombo();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JuiceLab|Dummy")
    int32 CurrentCombo = 0;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> TorsoDynMat;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> HeadDynMat;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DiscDynMat;

    FTweenHandle PunchScaleHandle;
    FTweenHandle PunchPositionHandle;
    FTweenHandle WobbleTweenHandle;
    FTweenHandle FlashTweenHandle;
    FTweenHandle DiscScaleTweenHandle;

    double LastHitTime = -10.0;
};
