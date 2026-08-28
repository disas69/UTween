#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TweenHandle.h"
#include "JuiceChestActor.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChestStateChangedSignature, bool, bIsOpen);

UCLASS(Blueprintable, BlueprintType)
class UTWEENPROJECT_API AJuiceChestActor : public AActor
{
    GENERATED_BODY()

public:
    AJuiceChestActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ChestBase;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> LidHinge;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ChestLid;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> LockGem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> LootContainer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TObjectPtr<UStaticMeshComponent>> LootItems;

    UPROPERTY(BlueprintAssignable, Category = "JuiceLab|Chest")
    FOnChestStateChangedSignature OnChestStateChanged;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JuiceLab|Chest")
    bool bIsOpen = false;

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Chest")
    void ToggleChest();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Chest")
    void OpenChest();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Chest")
    void CloseChest();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> LockDynMat;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> LootDynMats;

    TArray<FTweenHandle> LootSpinHandles;

    bool bIsAnimating = false;
};
