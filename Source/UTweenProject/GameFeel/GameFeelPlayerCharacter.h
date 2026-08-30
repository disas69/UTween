#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GameFeelPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class USceneComponent;
class AGameFeelSlashArc;
class UInputAction;
class UInputMappingContext;

UCLASS()
class UTWEENPROJECT_API AGameFeelPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AGameFeelPlayerCharacter();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PawnClientRestart() override;

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

    void TriggerHitstop(float Duration = 0.055f);

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Camera")
    void AddCameraTrauma(float Amount);

    UFUNCTION(BlueprintCallable, Category = "GameFeel|Camera")
    void TriggerCameraPunchZoom(float TargetZoomLength = 790.f, float ZoomInDuration = 0.05f, float ReturnDuration = 0.20f);

    void OnEnemyKilledByPlayer();

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Visuals")
    TSubclassOf<AGameFeelSlashArc> SlashArcClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Input")
    TObjectPtr<UInputAction> AimAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Input")
    TObjectPtr<UInputAction> AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Input")
    TObjectPtr<UInputAction> TogglePolishAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Camera")
    float DefaultBoomLength = 850.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Camera")
    float TraumaDecaySpeed = 3.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Camera")
    float MaxYawShake = 2.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Camera")
    float MaxPitchShake = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Camera")
    float MaxRollShake = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Camera")
    float MaxPositionalShake = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Camera")
    float ShakeFrequency = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Stats")
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameFeel|Stats")
    float CurrentHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Combat")
    float AttackDamage = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Combat")
    float AttackDuration = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Combat")
    float AttackRange = 180.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Combat")
    float RestStanceAngle = 85.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Polish")
    bool bIsPolished = true;

protected:
    virtual void BeginPlay() override;

    void EnhancedMove(const FInputActionValue& Value);
    void EnhancedAim(const FInputActionValue& Value);
    void EnhancedAimCompleted(const FInputActionValue& Value);

    void AimAtCursor();
    void DetectAttackHits();
    void UpdateCameraDynamics(float DeltaTime);

private:
    void ExecutePolishedAttack();
    void ExecuteUnpolishedAttack();

    bool bIsAttacking = false;
    float AttackTimer = 0.f;
    float CurrentRestAngle = 85.f;
    float SwingFromAngle = 85.f;
    float SwingToAngle = -85.f;
    TSet<TWeakObjectPtr<AActor>> HitActorsThisSwing;

    bool bUsingGamepadAim = false;
    float CameraTrauma = 0.f;
    float TraumaTimer = 0.f;

    UPROPERTY(Transient)
    TObjectPtr<AGameFeelSlashArc> SlashArcInstance;
};