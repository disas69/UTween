#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFeelEnemyCharacter.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;

UCLASS()
class UTWEENPROJECT_API AGameFeelEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AGameFeelEnemyCharacter();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Enemy")
    void TakeHit(float Damage, const FVector& HitDirection, bool bIsCritical);

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Enemy")
    void ResetEnemy();

    UFUNCTION(BlueprintPure, Category = "GameFeel|Enemy")
    float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> VisualRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BodyCylinder;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> TopCapSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> BottomCapSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> NoseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Stats")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameFeel|Stats")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Stats")
    float MoveSpeed = 160.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Stats")
    float KnockbackForce = 900.f;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

protected:
    virtual void BeginPlay() override;

private:
    void FollowPlayer(float DeltaTime);
    void Die();

    FVector InitialSpawnLocation = FVector::ZeroVector;
    bool bIsDead = false;
};