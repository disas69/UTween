#include "GameFeelPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

#include "GameFeelEnemyCharacter.h"
#include "UTween.h"

AGameFeelPlayerCharacter::AGameFeelPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    if (CapsuleComp)
    {
        CapsuleComp->InitCapsuleSize(35.f, 65.f);
        CapsuleComp->SetCollisionProfileName(TEXT("Pawn"));
    }

    VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
    VisualRoot->SetupAttachment(RootComponent);
    VisualRoot->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_Player.MI_GameFeel_Player"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SwordMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_Sword.MI_GameFeel_Sword"));

    // Cylinder body of capsule
    BodyCylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyCylinder"));
    BodyCylinder->SetupAttachment(VisualRoot);
    BodyCylinder->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    BodyCylinder->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.6f));
    BodyCylinder->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CylinderFinder.Succeeded()) BodyCylinder->SetStaticMesh(CylinderFinder.Object);
    if (PlayerMatFinder.Succeeded()) BodyCylinder->SetMaterial(0, PlayerMatFinder.Object);

    // Top cap sphere
    TopCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopCapSphere"));
    TopCapSphere->SetupAttachment(VisualRoot);
    TopCapSphere->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
    TopCapSphere->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
    TopCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) TopCapSphere->SetStaticMesh(SphereFinder.Object);
    if (PlayerMatFinder.Succeeded()) TopCapSphere->SetMaterial(0, PlayerMatFinder.Object);

    // Bottom cap sphere
    BottomCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomCapSphere"));
    BottomCapSphere->SetupAttachment(VisualRoot);
    BottomCapSphere->SetRelativeLocation(FVector(0.f, 0.f, -30.f));
    BottomCapSphere->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
    BottomCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) BottomCapSphere->SetStaticMesh(SphereFinder.Object);
    if (PlayerMatFinder.Succeeded()) BottomCapSphere->SetMaterial(0, PlayerMatFinder.Object);

    // White Nose to clearly indicate forward facing direction
    NoseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoseMesh"));
    NoseMesh->SetupAttachment(VisualRoot);
    NoseMesh->SetRelativeLocation(FVector(32.f, 0.f, 15.f));
    NoseMesh->SetRelativeScale3D(FVector(0.24f, 0.16f, 0.16f));
    NoseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) NoseMesh->SetStaticMesh(SphereFinder.Object);
    if (SwordMatFinder.Succeeded()) NoseMesh->SetMaterial(0, SwordMatFinder.Object);

    // Camera Boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->TargetArmLength = 850.f;
    CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritYaw = false;
    CameraBoom->bInheritRoll = false;

    // Top-Down Camera
    TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TopDownCamera->bUsePawnControlRotation = false;

    // Sword Pivot & Mesh
    SwordPivot = CreateDefaultSubobject<USceneComponent>(TEXT("SwordPivot"));
    SwordPivot->SetupAttachment(RootComponent);
    SwordPivot->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

    SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
    SwordMesh->SetupAttachment(SwordPivot);
    SwordMesh->SetRelativeLocation(FVector(55.f, 0.f, 0.f));
    SwordMesh->SetRelativeScale3D(FVector(0.75f, 0.16f, 0.08f));
    SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CubeFinder.Succeeded()) SwordMesh->SetStaticMesh(CubeFinder.Object);
    if (SwordMatFinder.Succeeded()) SwordMesh->SetMaterial(0, SwordMatFinder.Object);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = false;
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->MaxWalkSpeed = 580.f;
        MoveComp->BrakingDecelerationWalking = 2400.f;
    }

    CurrentHealth = MaxHealth;
    CurrentRestAngle = RestStanceAngle;
}

void AGameFeelPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    CurrentRestAngle = RestStanceAngle;
    if (SwordPivot)
    {
        SwordPivot->SetRelativeRotation(FRotator(0.f, CurrentRestAngle, 0.f));
    }
}

void AGameFeelPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AimAtCursor();

    if (bIsAttacking)
    {
        AttackTimer += DeltaTime;
        const float Alpha = FMath::Clamp(AttackTimer / AttackDuration, 0.f, 1.f);
        const float CurrentAngle = FMath::Lerp(SwingFromAngle, SwingToAngle, Alpha);

        if (SwordPivot)
        {
            SwordPivot->SetRelativeRotation(FRotator(0.f, CurrentAngle, 0.f));
        }

        DetectAttackHits();

        if (AttackTimer >= AttackDuration)
        {
            bIsAttacking = false;
            CurrentRestAngle = SwingToAngle;
            if (SwordPivot)
            {
                SwordPivot->SetRelativeRotation(FRotator(0.f, CurrentRestAngle, 0.f));
            }
        }
    }
}

void AGameFeelPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (PlayerInputComponent)
    {
        PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AGameFeelPlayerCharacter::MoveForward);
        PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AGameFeelPlayerCharacter::MoveRight);
        PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &AGameFeelPlayerCharacter::PerformAttack);
        PlayerInputComponent->BindAction(TEXT("TogglePolish"), IE_Pressed, this, &AGameFeelPlayerCharacter::TogglePolishMode);
    }
}

void AGameFeelPlayerCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(FVector::ForwardVector, Value);
    }
}

void AGameFeelPlayerCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(FVector::RightVector, Value);
    }
}

void AGameFeelPlayerCharacter::AimAtCursor()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    FHitResult HitResult;
    if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
    {
        FVector TargetLocation = HitResult.ImpactPoint;
        TargetLocation.Z = GetActorLocation().Z;

        FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
        if (!Direction.IsNearlyZero())
        {
            SetActorRotation(FRotator(0.f, Direction.Rotation().Yaw, 0.f));
        }
    }
}

void AGameFeelPlayerCharacter::PerformAttack()
{
    if (bIsAttacking)
    {
        return;
    }

    bIsAttacking = true;
    AttackTimer = 0.f;
    HitActorsThisSwing.Empty();

    // Swing from current resting side to opposite resting side
    SwingFromAngle = CurrentRestAngle;
    SwingToAngle = -CurrentRestAngle;
}

void AGameFeelPlayerCharacter::DetectAttackHits()
{
    UWorld* World = GetWorld();
    if (!World || !SwordMesh)
    {
        return;
    }

    const FVector SwordLocation = SwordMesh->GetComponentLocation();
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRange * 0.5f);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        SwordLocation,
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        Params);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (AGameFeelEnemyCharacter* Enemy = Cast<AGameFeelEnemyCharacter>(HitActor))
        {
            if (!HitActorsThisSwing.Contains(Enemy))
            {
                HitActorsThisSwing.Add(Enemy);
                const FVector HitDir = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                Enemy->TakeHit(AttackDamage, HitDir, false);
            }
        }
    }
}

void AGameFeelPlayerCharacter::TogglePolishMode()
{
    SetPolishMode(!bIsPolished);
}

void AGameFeelPlayerCharacter::SetPolishMode(bool bEnable)
{
    bIsPolished = bEnable;
}

void AGameFeelPlayerCharacter::TakePlayerDamage(float Damage)
{
    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
}