#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFeelImpactSparks.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS(Blueprintable)
class UTWEENPROJECT_API AGameFeelImpactSparks : public AActor
{
    GENERATED_BODY()

public:
    AGameFeelImpactSparks();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Sparks")
    virtual void TriggerSparks(const FVector& HitDirection);

    UFUNCTION(BlueprintImplementableEvent, Category = "GameFeel|Sparks")
    void OnSparksTriggered(const FVector& HitDirection);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Sparks")
    int32 SparkCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Sparks")
    float SparkLifetime = 0.22f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Sparks")
    float MinSpeed = 450.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Sparks")
    float MaxSpeed = 850.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Sparks")
    FVector SparkScale = FVector(0.12f, 0.12f, 0.12f);

private:
    struct FSparkParticle
    {
        TWeakObjectPtr<UStaticMeshComponent> MeshComp;
        FVector Velocity = FVector::ZeroVector;
    };

    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> SparkMeshes;

    TArray<FSparkParticle> Particles;
    float ElapsedLifetime = 0.f;
    bool bIsActive = false;
};