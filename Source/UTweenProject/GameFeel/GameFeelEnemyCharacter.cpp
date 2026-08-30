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

#include "GameFeelDamageNumber.h"
#include "GameFeelImpactSparks.h"
#include "GameFeelPlayerCharacter.h"
#include "UTween.h"

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

    NoseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoseMesh"));
    NoseMesh->SetupAttachment(VisualRoot);
    NoseMesh->SetRelativeLocation(FVector(27.f, 0.f, 12.f));
    NoseMesh->SetRelativeScale3D(FVector(0.20f, 0.14f, 0.14f));
    NoseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) NoseMesh->SetStaticMesh(SphereFinder.Object);
    if (SwordMatFinder.Succeeded()) NoseMesh->SetMaterial(0, SwordMatFinder.Object);

    static ConstructorHelpers::FClassFinder<AGameFeelImpactSparks> SparksBP(TEXT("/Game/GameFeel/Blueprints/BP_GameFeelImpactSparks.BP_GameFeelImpactSparks_C"));
    if (SparksBP.Succeeded())
    {
        ImpactSparksClass = SparksBP.Class;
    }
    else
    {
        ImpactSparksClass = AGameFeelImpactSparks::StaticClass();
    }

    static ConstructorHelpers::FClassFinder<AGameFeelDamageNumber> DmgBP(TEXT("/Game/GameFeel/Blueprints/BP_GameFeelDamageNumber.BP_GameFeelDamageNumber_C"));
    if (DmgBP.Succeeded())
    {
        DamageNumberClass = DmgBP.Class;
    }
    else
    {
        DamageNumberClass = AGameFeelDamageNumber::StaticClass();
    }

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

        const FRotator TargetRot = Direction.Rotation();
        SetActorRotation(FRotator(0.f, TargetRot.Yaw, 0.f));

        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->AddInputVector(Direction, true);
        }
        AddMovementInput(Direction, 1.0f);
    }
}

void AGameFeelEnemyCharacter::TakeHit(float Damage, const FVector& HitDirection, bool bPolishedHit)
{
    if (bIsDead)
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);

    if (!bPolishedHit)
    {
        const FVector LaunchVel = HitDirection * KnockbackForce + FVector(0.f, 0.f, 80.f);
        LaunchCharacter(LaunchVel, true, true);
    }
    else
    {
        const FVector LaunchVel = HitDirection * (KnockbackForce * 1.3f) + FVector(0.f, 0.f, 120.f);
        LaunchCharacter(LaunchVel, true, true);

        // 1. Emissive White Hit Flash via UTween
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor(6.0f, 6.0f, 6.0f, 1.0f));
            UTween::MaterialVector(DynamicMaterial, FName("EmissiveColor"), FLinearColor::Black, 0.14f)
                .SetEase(ETweenEase::OutQuad);
        }

        // 2. Squash & Stretch Elasticity via UTween
        if (VisualRoot)
        {
            VisualRoot->SetRelativeScale3D(FVector(1.4f, 1.4f, 0.55f));
            UTween::ComponentRelativeScale(VisualRoot, FVector(1.0f, 1.0f, 1.0f), 0.28f)
                .SetEase(ETweenEase::OutElastic);
        }

        // 3. Spawn native Impact Sparks Blueprint actor
        if (ImpactSparksClass && GetWorld())
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            if (AGameFeelImpactSparks* Sparks = GetWorld()->SpawnActor<AGameFeelImpactSparks>(ImpactSparksClass, GetActorLocation() + FVector(0.f, 0.f, 15.f), HitDirection.Rotation(), Params))
            {
                Sparks->TriggerSparks(HitDirection);
            }
        }

        // 4. Spawn Floating Damage Number via UTween
        if (DamageNumberClass && GetWorld())
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            if (AGameFeelDamageNumber* DmgNum = GetWorld()->SpawnActor<AGameFeelDamageNumber>(DamageNumberClass, GetActorLocation(), FRotator::ZeroRotator, Params))
            {
                DmgNum->InitializeDamage(Damage, GetActorLocation());
            }
        }
    }

    if (CurrentHealth <= 0.f)
    {
        if (bPolishedHit)
        {
            if (AGameFeelPlayerCharacter* Player = Cast<AGameFeelPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
            {
                Player->OnEnemyKilledByPlayer();
            }
        }
        Die();
    }
}

void AGameFeelEnemyCharacter::Die()
{
    bIsDead = true;
    SetActorEnableCollision(false);

    if (AGameFeelPlayerCharacter* Player = Cast<AGameFeelPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        if (Player->IsPolishEnabled() && VisualRoot)
        {
            // Polished 3-step death animation sequence: Flatten -> Pop Up -> Shrink / Implode
            UTween::ComponentRelativeScale(VisualRoot, FVector(1.6f, 1.6f, 0.2f), 0.07f)
                .SetEase(ETweenEase::OutQuad)
                .OnComplete([this]() {
                    if (VisualRoot)
                    {
                        UTween::ComponentRelativeLocation(VisualRoot, FVector(0.f, 0.f, 60.f), 0.12f)
                            .SetEase(ETweenEase::OutBack);

                        UTween::ComponentRelativeScale(VisualRoot, FVector(0.7f, 0.7f, 1.5f), 0.12f)
                            .SetEase(ETweenEase::OutBack)
                            .OnComplete([this]() {
                                if (VisualRoot)
                                {
                                    UTween::ComponentRelativeScale(VisualRoot, FVector(0.01f, 0.01f, 0.01f), 0.10f)
                                        .SetEase(ETweenEase::InBack)
                                        .OnComplete([this]() {
                                            SetActorHiddenInGame(true);
                                        });
                                }
                            });
                    }
                });

            FTimerHandle RespawnTimer;
            GetWorldTimerManager().SetTimer(
                RespawnTimer,
                this,
                &AGameFeelEnemyCharacter::ResetEnemy,
                3.0f,
                false);
            return;
        }
    }

    SetActorHiddenInGame(true);
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

    if (VisualRoot)
    {
        VisualRoot->SetRelativeLocation(FVector::ZeroVector);
        VisualRoot->SetRelativeScale3D(FVector::OneVector);
    }
    if (DynamicMaterial)
    {
        DynamicMaterial->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Black);
    }
}