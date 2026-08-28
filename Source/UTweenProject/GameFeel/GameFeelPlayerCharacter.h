#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFeelPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class UTWEENPROJECT_API AGameFeelPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AGameFeelPlayerCharacter();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Combat")
    void TogglePolishMode();

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Combat")
    void SetPolishMode(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "GameFeel|Combat")
    bool IsPolishEnabled() const { return bIsPolished; }

    UFUNCTION(BlueprintPure, Category = "GameFeel|Combat")
    bool IsAttacking() const { return bIsAttacking; }

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Combat")
    void TakePlayerDamage(float Damage);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCameraComponent> TopDownCamera;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SwordPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> SwordMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Stats")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameFeel|Stats")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Combat")
    float AttackDamage = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Combat")
    float AttackDuration = 0.22f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Combat")
    float AttackRange = 180.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Combat")
    float RestStanceAngle = 85.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Polish")
    bool bIsPolished = false;

protected:
    virtual void BeginPlay() override;

    void MoveForward(float Value);
    void MoveRight(float Value);
    void AimAtCursor();
    void DetectAttackHits();

private:
    bool bIsAttacking = false;
    float AttackTimer = 0.f;
    float CurrentRestAngle = 85.f;
    float SwingFromAngle = 85.f;
    float SwingToAngle = -85.f;
    TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;
};