#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFeelSlashArc.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;

UCLASS(Blueprintable)
class UTWEENPROJECT_API AGameFeelSlashArc : public AActor
{
    GENERATED_BODY()

public:
    AGameFeelSlashArc();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Slash")
    virtual void TriggerSlash(float Duration, FLinearColor CustomSlashColor = FLinearColor(-1.f, -1.f, -1.f, 1.f));

    UFUNCTION(BlueprintImplementableEvent, Category = "GameFeel|Slash")
    void OnSlashTriggered(float Duration);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ArcMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Slash")
    FVector MaxSlashScale = FVector(1.22f, 1.22f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Slash")
    FLinearColor SlashEmissiveColor = FLinearColor(9.5f, 7.8f, 5.2f, 1.0f);

protected:
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicSlashMaterial;
};