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

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CapsuleFinder(TEXT("/Game/GameFeel/Meshes/SM_GameFeel_Capsule.SM_GameFeel_Capsule"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> EnemyMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_Enemy.MI_GameFeel_Enemy"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DissolveMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_EnemyDissolve.MI_GameFeel_EnemyDissolve"));
    if (DissolveMatFinder.Succeeded())
    {
        DissolveMaterial = DissolveMatFinder.Object;
    }

    BodyCylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyCylinder"));
    BodyCylinder->SetupAttachment(VisualRoot);
    BodyCylinder->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    BodyCylinder->SetRelativeScale3D(FVector(0.92f, 0.92f, 0.92f));
    BodyCylinder->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CapsuleFinder.Succeeded())
    {
        BodyCylinder->SetStaticMesh(CapsuleFinder.Object);
    }
    else if (CylinderFinder.Succeeded())
    {
        BodyCylinder->SetStaticMesh(CylinderFinder.Object);
        BodyCylinder->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.5f));
    }
    if (EnemyMatFinder.Succeeded()) BodyCylinder->SetMaterial(0, EnemyMatFinder.Object);

    TopCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopCapSphere"));
    TopCapSphere->SetupAttachment(VisualRoot);
    TopCapSphere->SetRelativeLocation(FVector(0.f, 0.f, 25.f));
    TopCapSphere->SetRelativeScale3D(FVector(0.001f, 0.001f, 0.001f));
    TopCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    TopCapSphere->SetVisibility(false);
    TopCapSphere->SetHiddenInGame(true);
    if (SphereFinder.Succeeded()) TopCapSphere->SetStaticMesh(SphereFinder.Object);
    if (EnemyMatFinder.Succeeded()) TopCapSphere->SetMaterial(0, EnemyMatFinder.Object);

    BottomCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomCapSphere"));
    BottomCapSphere->SetupAttachment(VisualRoot);
    BottomCapSphere->SetRelativeLocation(FVector(0.f, 0.f, -25.f));
    BottomCapSphere->SetRelativeScale3D(FVector(0.001f, 0.001f, 0.001f));
    BottomCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BottomCapSphere->SetVisibility(false);
    BottomCapSphere->SetHiddenInGame(true);
    if (SphereFinder.Succeeded()) BottomCapSphere->SetStaticMesh(SphereFinder.Object);
    if (EnemyMatFinder.Succeeded()) BottomCapSphere->SetMaterial(0, EnemyMatFinder.Object);

    NoseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoseMesh"));
    NoseMesh->SetupAttachment(VisualRoot);
    NoseMesh->SetRelativeLocation(FVector(25.f, 0.f, 12.f));
    NoseMesh->SetRelativeScale3D(FVector(0.22f, 0.22f, 0.22f));
    NoseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) NoseMesh->SetStaticMesh(SphereFinder.Object);
    if (EnemyMatFinder.Succeeded()) NoseMesh->SetMaterial(0, EnemyMatFinder.Object);

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
        if (NoseMesh && DynamicMaterial) NoseMesh->SetMaterial(0, DynamicMaterial);
    }

    if (DissolveMaterial)
    {
        DissolveDynamicMaterial = UMaterialInstanceDynamic::Create(DissolveMaterial, this);
    }

    StartIdleAnimation();
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
    const float Dist2D = Diff.Size2D();
    const FVector ToPlayerDir = FVector(Diff.X, Diff.Y, 0.f).GetSafeNormal();

    if (!ToPlayerDir.IsNearlyZero())
    {
        const FRotator TargetRot(0.f, ToPlayerDir.Rotation().Yaw, 0.f);
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 12.f));
    }

    FVector MoveVector = FVector::ZeroVector;
    if (Dist2D > 235.f)
    {
        MoveVector += ToPlayerDir;
    }
    else if (Dist2D < 82.f)
    {
        MoveVector -= ToPlayerDir * 0.6f;
    }

    // Keep enemies spread in a clean semi-circle / ring around the player
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsOfClass(this, AGameFeelEnemyCharacter::StaticClass(), AllEnemies);
    for (AActor* Other : AllEnemies)
    {
        if (Other && Other != this && !Other->IsHidden())
        {
            const FVector Away = EnemyLoc - Other->GetActorLocation();
            const float OtherDist2D = Away.Size2D();
            if (OtherDist2D > 1.f && OtherDist2D < 120.f)
            {
                const float PushStrength = (120.f - OtherDist2D) / 120.f;
                MoveVector += FVector(Away.X, Away.Y, 0.f).GetSafeNormal() * (PushStrength * 1.15f);
            }
        }
    }

    if (!MoveVector.IsNearlyZero())
    {
        AddMovementInput(MoveVector.GetSafeNormal(), 1.0f);
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
    else
    {
        const FVector LaunchVel = HitDirection * (KnockbackForce * 1.3f) + FVector(0.f, 0.f, 120.f);
        LaunchCharacter(LaunchVel, true, true);

        StopIdleAnimation();
        HitScaleHandle.Kill();
        HitRotHandle.Kill();
        HitFlashHandle.Kill();

        // 1. Emissive Warm Golden-Peach Hit Flash via UTween
        if (DynamicMaterial)
        {
            HitFlashHandle = UTween::MaterialVector(DynamicMaterial, FName("EmissiveColor"), FLinearColor::Black, 0.18f)
                .From(FLinearColor(5.5f, 3.2f, 1.4f, 1.0f))
                .SetEase(ETweenEase::OutQuad);
        }

        // 2. Squash & Stretch Elasticity + Angular Flinch Recoil via UTween
        if (VisualRoot)
        {
            const FRotator FlinchRot(
                FMath::RandRange(-12.f, 12.f),
                FMath::RandRange(-18.f, 18.f),
                FMath::RandRange(-8.f, 8.f));

            TWeakObjectPtr<AGameFeelEnemyCharacter> WeakThis(this);
            HitScaleHandle = UTween::ComponentRelativeScale(VisualRoot, FVector::OneVector, 0.28f)
                .From(FVector(0.84f, 0.84f, 1.38f))
                .SetEase(ETweenEase::OutElastic)
                .OnComplete([WeakThis]() {
                    if (AGameFeelEnemyCharacter* Self = WeakThis.Get())
                    {
                        if (!Self->bIsDead)
                        {
                            Self->StartIdleAnimation();
                        }
                    }
                });

            HitRotHandle = UTween::ComponentRelativeRotation(VisualRoot, FRotator::ZeroRotator, 0.32f)
                .From(FlinchRot)
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

    StopIdleAnimation();
    HitScaleHandle.Kill();
    HitRotHandle.Kill();
    HitFlashHandle.Kill();
    DeathSequenceHandle.Kill();

    if (AGameFeelPlayerCharacter* Player = Cast<AGameFeelPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        if (Player->IsPolishEnabled() && VisualRoot)
        {
            // 1. Switch to Dissolve Material and animate DissolveAmount
            if (!DissolveDynamicMaterial && DissolveMaterial)
            {
                DissolveDynamicMaterial = UMaterialInstanceDynamic::Create(DissolveMaterial, this);
            }
            if (DissolveDynamicMaterial)
            {
                DissolveDynamicMaterial->SetScalarParameterValue(FName("DissolveAmount"), 0.0f);
                if (BodyCylinder) BodyCylinder->SetMaterial(0, DissolveDynamicMaterial);
                if (TopCapSphere) TopCapSphere->SetMaterial(0, DissolveDynamicMaterial);
                if (BottomCapSphere) BottomCapSphere->SetMaterial(0, DissolveDynamicMaterial);
                if (NoseMesh) NoseMesh->SetMaterial(0, DissolveDynamicMaterial);

                UTween::MaterialScalar(DissolveDynamicMaterial, FName("DissolveAmount"), 1.0f, 0.35f)
                    .From(0.0f)
                    .SetEase(ETweenEase::InQuad);
            }

            // 2. Polished 3-step death choreography via FTweenSequence: Squash -> Pop Up -> Shrink / Implode
            TWeakObjectPtr<AGameFeelEnemyCharacter> WeakThis(this);
            DeathSequenceHandle = UTween::Sequence()
                .SetBoundTarget(this)
                .Append(UTween::ComponentRelativeScale(VisualRoot, FVector(1.6f, 1.6f, 0.2f), 0.07f)
                    .SetEase(ETweenEase::OutQuad))
                .Append(UTween::ComponentRelativeLocation(VisualRoot, FVector(0.f, 0.f, 60.f), 0.12f)
                    .SetEase(ETweenEase::OutBack))
                .Join(UTween::ComponentRelativeScale(VisualRoot, FVector(0.7f, 0.7f, 1.5f), 0.12f)
                    .From(FVector(1.6f, 1.6f, 0.2f))
                    .SetEase(ETweenEase::OutBack))
                .Append(UTween::ComponentRelativeScale(VisualRoot, FVector(0.01f, 0.01f, 0.01f), 0.14f)
                    .From(FVector(0.7f, 0.7f, 1.5f))
                    .SetEase(ETweenEase::InBack))
                .OnComplete([WeakThis]() {
                    if (AGameFeelEnemyCharacter* Self = WeakThis.Get())
                    {
                        Self->SetActorHiddenInGame(true);
                    }
                })
                .Play();

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

    StopIdleAnimation();
    HitScaleHandle.Kill();
    HitRotHandle.Kill();
    HitFlashHandle.Kill();
    DeathSequenceHandle.Kill();

    if (VisualRoot)
    {
        VisualRoot->SetRelativeLocation(FVector::ZeroVector);
        VisualRoot->SetRelativeRotation(FRotator::ZeroRotator);
        VisualRoot->SetRelativeScale3D(FVector::OneVector);
    }
    if (DynamicMaterial)
    {
        if (BodyCylinder) BodyCylinder->SetMaterial(0, DynamicMaterial);
        if (TopCapSphere) TopCapSphere->SetMaterial(0, DynamicMaterial);
        if (BottomCapSphere) BottomCapSphere->SetMaterial(0, DynamicMaterial);
        if (NoseMesh) NoseMesh->SetMaterial(0, DynamicMaterial);
        DynamicMaterial->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Black);
    }

    StartIdleAnimation();
}

void AGameFeelEnemyCharacter::StartIdleAnimation()
{
    if (AGameFeelPlayerCharacter* Player = Cast<AGameFeelPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        if (!Player->IsPolishEnabled())
        {
            return;
        }
    }

    if (VisualRoot && !bIsDead)
    {
        IdleTweenHandle.Kill();
        IdleTweenHandle = UTween::ComponentRelativeScale(VisualRoot, FVector(1.04f, 1.04f, 0.95f), 0.75f)
            .From(FVector::OneVector)
            .SetEase(ETweenEase::InOutSine)
            .SetLoops(-1, ETweenLoopMode::Yoyo)
            .SetTag(FTweenTag(TEXT("EnemyIdle")));
    }
}

void AGameFeelEnemyCharacter::StopIdleAnimation()
{
    IdleTweenHandle.Kill();
}
