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

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    if (CapsuleComp)
    {
        CapsuleComp->InitCapsuleSize(30.f, 55.f);
        CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
    }

    VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
    VisualRoot->SetupAttachment(RootComponent);
    VisualRoot->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> EnemyMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_Enemy.MI_GameFeel_Enemy"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SwordMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_Sword.MI_GameFeel_Sword"));

    BodyCylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyCylinder"));
    BodyCylinder->SetupAttachment(VisualRoot);
    BodyCylinder->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    BodyCylinder->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.5f));
    BodyCylinder->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CylinderFinder.Succeeded()) BodyCylinder->SetStaticMesh(CylinderFinder.Object);
    if (EnemyMatFinder.Succeeded()) BodyCylinder->SetMaterial(0, EnemyMatFinder.Object);

    TopCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopCapSphere"));
    TopCapSphere->SetupAttachment(VisualRoot);
    TopCapSphere->SetRelativeLocation(FVector(0.f, 0.f, 25.f));
    TopCapSphere->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
    TopCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) TopCapSphere->SetStaticMesh(SphereFinder.Object);
    if (EnemyMatFinder.Succeeded()) TopCapSphere->SetMaterial(0, EnemyMatFinder.Object);

    BottomCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomCapSphere"));
    BottomCapSphere->SetupAttachment(VisualRoot);
    BottomCapSphere->SetRelativeLocation(FVector(0.f, 0.f, -25.f));
    BottomCapSphere->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
    BottomCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) BottomCapSphere->SetStaticMesh(SphereFinder.Object);
    if (EnemyMatFinder.Succeeded()) BottomCapSphere->SetMaterial(0, EnemyMatFinder.Object);

    // White Nose to indicate forward facing direction
    NoseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoseMesh"));
    NoseMesh->SetupAttachment(VisualRoot);
    NoseMesh->SetRelativeLocation(FVector(27.f, 0.f, 12.f));
    NoseMesh->SetRelativeScale3D(FVector(0.20f, 0.14f, 0.14f));
    NoseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) NoseMesh->SetStaticMesh(SphereFinder.Object);
    if (SwordMatFinder.Succeeded()) NoseMesh->SetMaterial(0, SwordMatFinder.Object);

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = MoveSpeed;
        MoveComp->bOrientRotationToMovement = false;
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->BrakingDecelerationWalking = 1200.f;
    }

    CurrentHealth = MaxHealth;
}

void AGameFeelEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitialSpawnLocation = GetActorLocation();

    if (BodyCylinder)
    {
        DynamicMaterial = BodyCylinder->CreateAndSetMaterialInstanceDynamic(0);
        if (TopCapSphere && DynamicMaterial) TopCapSphere->SetMaterial(0, DynamicMaterial);
        if (BottomCapSphere && DynamicMaterial) BottomCapSphere->SetMaterial(0, DynamicMaterial);
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

    if (DistSq > FMath::Square(55.f))
    {
        const FVector Direction = FVector(Diff.X, Diff.Y, 0.f).GetSafeNormal();

        // Face the player
        const FRotator TargetRot = Direction.Rotation();
        SetActorRotation(FRotator(0.f, TargetRot.Yaw, 0.f));

        // Directly feed input into CharacterMovementComponent
        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->AddInputVector(Direction, true);
        }
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

    // Natural physics knockback push
    const FVector LaunchVel = HitDirection * KnockbackForce + FVector(0.f, 0.f, 80.f);
    LaunchCharacter(LaunchVel, true, true);

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
        3.0f,
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