#include "GameFeelEnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

#include "GameFeelPlayerCharacter.h"

AGameFeelEnemyCharacter::AGameFeelEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    if (CapsuleComp)
    {
        CapsuleComp->InitCapsuleSize(30.f, 55.f);
        CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
    }

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -55.f));
    VisualMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.1f));
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderFinder.Succeeded())
    {
        VisualMesh->SetStaticMesh(CylinderFinder.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> EnemyMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_Enemy.MI_GameFeel_Enemy"));
    if (EnemyMatFinder.Succeeded())
    {
        VisualMesh->SetMaterial(0, EnemyMatFinder.Object);
    }

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = MoveSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 360.f, 0.f);
    }

    CurrentHealth = MaxHealth;
}

void AGameFeelEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitialSpawnLocation = GetActorLocation();

    if (VisualMesh)
    {
        DynamicMaterial = VisualMesh->CreateAndSetMaterialInstanceDynamic(0);
    }
}

void AGameFeelEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsDead)
    {
        FollowPlayer(DeltaTime);
    }
}

void AGameFeelEnemyCharacter::FollowPlayer(float DeltaTime)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn)
    {
        return;
    }

    const FVector PlayerLoc = PlayerPawn->GetActorLocation();
    const FVector EnemyLoc = GetActorLocation();
    const FVector Diff = PlayerLoc - EnemyLoc;
    const float DistSq = Diff.SizeSquared2D();

    // Stop at attack range
    if (DistSq > FMath::Square(75.f))
    {
        const FVector Direction = FVector(Diff.X, Diff.Y, 0.f).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);
    }
}

void AGameFeelEnemyCharacter::TakeHit(float Damage, const FVector& HitDirection, bool bIsCritical)
{
    if (bIsDead)
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);

    if (CurrentHealth <= 0.f)
    {
        Die();
    }
}

void AGameFeelEnemyCharacter::Die()
{
    bIsDead = true;
    SetActorEnableCollision(false);
    SetActorHiddenInGame(true);

    // Respawn after short delay
    FTimerHandle RespawnTimer;
    GetWorldTimerManager().SetTimer(
        RespawnTimer,
        this,
        &AGameFeelEnemyCharacter::ResetEnemy,
        2.5f,
        false);
}

void AGameFeelEnemyCharacter::ResetEnemy()
{
    bIsDead = false;
    CurrentHealth = MaxHealth;
    SetActorLocation(InitialSpawnLocation);
    SetActorEnableCollision(true);
    SetActorHiddenInGame(false);
}