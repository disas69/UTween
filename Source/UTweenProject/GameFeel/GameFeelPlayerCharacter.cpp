#include "GameFeelPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

#include "GameFeelEnemyCharacter.h"
#include "GameFeelSlashArc.h"
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

    BodyCylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyCylinder"));
    BodyCylinder->SetupAttachment(VisualRoot);
    BodyCylinder->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    BodyCylinder->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.6f));
    BodyCylinder->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CylinderFinder.Succeeded()) BodyCylinder->SetStaticMesh(CylinderFinder.Object);
    if (PlayerMatFinder.Succeeded()) BodyCylinder->SetMaterial(0, PlayerMatFinder.Object);

    TopCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopCapSphere"));
    TopCapSphere->SetupAttachment(VisualRoot);
    TopCapSphere->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
    TopCapSphere->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
    TopCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) TopCapSphere->SetStaticMesh(SphereFinder.Object);
    if (PlayerMatFinder.Succeeded()) TopCapSphere->SetMaterial(0, PlayerMatFinder.Object);

    BottomCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomCapSphere"));
    BottomCapSphere->SetupAttachment(VisualRoot);
    BottomCapSphere->SetRelativeLocation(FVector(0.f, 0.f, -30.f));
    BottomCapSphere->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.7f));
    BottomCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) BottomCapSphere->SetStaticMesh(SphereFinder.Object);
    if (PlayerMatFinder.Succeeded()) BottomCapSphere->SetMaterial(0, PlayerMatFinder.Object);

    NoseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoseMesh"));
    NoseMesh->SetupAttachment(VisualRoot);
    NoseMesh->SetRelativeLocation(FVector(32.f, 0.f, 15.f));
    NoseMesh->SetRelativeScale3D(FVector(0.24f, 0.16f, 0.16f));
    NoseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) NoseMesh->SetStaticMesh(SphereFinder.Object);
    if (SwordMatFinder.Succeeded()) NoseMesh->SetMaterial(0, SwordMatFinder.Object);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->TargetArmLength = DefaultBoomLength;
    CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritYaw = false;
    CameraBoom->bInheritRoll = false;

    TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TopDownCamera->bUsePawnControlRotation = false;

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

    static ConstructorHelpers::FClassFinder<AGameFeelSlashArc> SlashBP(TEXT("/Game/GameFeel/Blueprints/BP_GameFeelSlashArc.BP_GameFeelSlashArc_C"));
    if (SlashBP.Succeeded())
    {
        SlashArcClass = SlashBP.Class;
    }
    else
    {
        SlashArcClass = AGameFeelSlashArc::StaticClass();
    }

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

    if (SlashArcClass && GetWorld())
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        SlashArcInstance = GetWorld()->SpawnActor<AGameFeelSlashArc>(SlashArcClass, GetActorLocation(), GetActorRotation(), Params);
        if (SlashArcInstance)
        {
            SlashArcInstance->AttachToComponent(VisualRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        }
    }
}

void AGameFeelPlayerCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->ClearAllMappings();
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
}

void AGameFeelPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AimAtCursor();
    UpdateCameraDynamics(DeltaTime);

    if (bIsAttacking && !bIsPolished)
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

    if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInputComp->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGameFeelPlayerCharacter::EnhancedMove);
        }
        if (AimAction)
        {
            EnhancedInputComp->BindAction(AimAction, ETriggerEvent::Triggered, this, &AGameFeelPlayerCharacter::EnhancedAim);
            EnhancedInputComp->BindAction(AimAction, ETriggerEvent::Completed, this, &AGameFeelPlayerCharacter::EnhancedAimCompleted);
        }
        if (AttackAction)
        {
            EnhancedInputComp->BindAction(AttackAction, ETriggerEvent::Started, this, &AGameFeelPlayerCharacter::PerformAttack);
        }
        if (TogglePolishAction)
        {
            EnhancedInputComp->BindAction(TogglePolishAction, ETriggerEvent::Started, this, &AGameFeelPlayerCharacter::TogglePolishMode);
        }
    }
}

void AGameFeelPlayerCharacter::EnhancedMove(const FInputActionValue& Value)
{
    const FVector2D MoveVec = Value.Get<FVector2D>();
    if (!MoveVec.IsNearlyZero())
    {
        AddMovementInput(FVector::ForwardVector, MoveVec.Y);
        AddMovementInput(FVector::RightVector, MoveVec.X);

        if (!bUsingGamepadAim)
        {
            const FVector MoveDir = FVector(MoveVec.Y, MoveVec.X, 0.f).GetSafeNormal();
            if (!MoveDir.IsNearlyZero())
            {
                SetActorRotation(FRotator(0.f, MoveDir.Rotation().Yaw, 0.f));
            }
        }
    }
}

void AGameFeelPlayerCharacter::EnhancedAim(const FInputActionValue& Value)
{
    const FVector2D AimVec = Value.Get<FVector2D>();
    if (AimVec.SizeSquared() > 0.05f)
    {
        bUsingGamepadAim = true;
        const FVector AimDir = FVector(AimVec.Y, AimVec.X, 0.f).GetSafeNormal();
        if (!AimDir.IsNearlyZero())
        {
            SetActorRotation(FRotator(0.f, AimDir.Rotation().Yaw, 0.f));
        }
    }
}

void AGameFeelPlayerCharacter::EnhancedAimCompleted(const FInputActionValue& Value)
{
    bUsingGamepadAim = false;
}

void AGameFeelPlayerCharacter::AimAtCursor()
{
    if (bUsingGamepadAim)
    {
        return;
    }

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

void AGameFeelPlayerCharacter::UpdateCameraDynamics(float DeltaTime)
{
    if (!TopDownCamera || !CameraBoom)
    {
        return;
    }

    if (!bIsPolished)
    {
        CameraTrauma = 0.f;
        TopDownCamera->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
        CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, FVector::ZeroVector, DeltaTime, 8.0f);
        CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, DefaultBoomLength, DeltaTime, 8.0f);
        return;
    }

    // Dynamic Look-Ahead Framing along Facing Vector
    const FVector TargetOffset = GetActorForwardVector() * 55.f;
    CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetOffset, DeltaTime, 4.5f);

    // Multi-Harmonic Rotational & Positional Camera Trauma Shake
    if (CameraTrauma > 0.001f)
    {
        CameraTrauma = FMath::Clamp(CameraTrauma - (TraumaDecaySpeed * DeltaTime), 0.f, 1.f);
        const float Shake = CameraTrauma * CameraTrauma;

        TraumaTimer += DeltaTime * ShakeFrequency;

        const float YawShake = FMath::Sin(TraumaTimer * 1.0f) * MaxYawShake * Shake;
        const float PitchShake = FMath::Cos(TraumaTimer * 1.37f) * MaxPitchShake * Shake;
        const float RollShake = FMath::Sin(TraumaTimer * 1.73f) * MaxRollShake * Shake;

        const FVector PosShake = FVector(
            FMath::Sin(TraumaTimer * 1.15f),
            FMath::Cos(TraumaTimer * 1.45f),
            FMath::Sin(TraumaTimer * 1.85f)
        ) * MaxPositionalShake * Shake;

        TopDownCamera->SetRelativeLocationAndRotation(PosShake, FRotator(PitchShake, YawShake, RollShake));
    }
    else
    {
        TopDownCamera->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
    }
}

void AGameFeelPlayerCharacter::AddCameraTrauma(float Amount)
{
    if (bIsPolished)
    {
        CameraTrauma = FMath::Clamp(CameraTrauma + Amount, 0.f, 1.f);
    }
}

void AGameFeelPlayerCharacter::TriggerCameraPunchZoom(float TargetZoomLength, float ZoomInDuration, float ReturnDuration)
{
    if (!bIsPolished || !CameraBoom)
    {
        return;
    }

    const float CurrentLength = CameraBoom->TargetArmLength;

    UTween::Value(this, CurrentLength, TargetZoomLength, ZoomInDuration, [this](float Length) {
        if (CameraBoom && bIsPolished)
        {
            CameraBoom->TargetArmLength = Length;
        }
    })
    .SetEase(ETweenEase::OutQuad)
    .OnComplete([this, TargetZoomLength, ReturnDuration]() {
        if (!bIsPolished || !CameraBoom)
        {
            return;
        }

        UTween::Value(this, TargetZoomLength, DefaultBoomLength, ReturnDuration, [this](float Length) {
            if (CameraBoom && bIsPolished)
            {
                CameraBoom->TargetArmLength = Length;
            }
        })
        .SetEase(ETweenEase::OutQuad);
    });
}

void AGameFeelPlayerCharacter::OnEnemyKilledByPlayer()
{
    if (bIsPolished)
    {
        AddCameraTrauma(0.55f);
        TriggerCameraPunchZoom(750.f, 0.06f, 0.25f);
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

    SwingFromAngle = CurrentRestAngle;
    SwingToAngle = -CurrentRestAngle;

    if (bIsPolished)
    {
        ExecutePolishedAttack();
    }
}

void AGameFeelPlayerCharacter::ExecutePolishedAttack()
{
    LaunchCharacter(GetActorForwardVector() * 360.f, false, false);
    AddCameraTrauma(0.10f);

    const float WindupSign = (SwingFromAngle > 0.f) ? 1.f : -1.f;
    const float WindupAngle = SwingFromAngle + (WindupSign * 18.f);
    const float OvershootAngle = SwingToAngle - (WindupSign * 12.f);

    UTween::Value(this, SwingFromAngle, WindupAngle, 0.045f, [this](float Angle) {
        if (SwordPivot)
        {
            SwordPivot->SetRelativeRotation(FRotator(0.f, Angle, 0.f));
        }
    })
    .SetEase(ETweenEase::OutQuad)
    .OnComplete([this, WindupAngle, OvershootAngle]() {
        if (SlashArcInstance)
        {
            SlashArcInstance->TriggerSlash(0.14f);
        }

        UTween::Value(this, WindupAngle, OvershootAngle, 0.12f, [this](float Angle) {
            if (SwordPivot)
            {
                SwordPivot->SetRelativeRotation(FRotator(0.f, Angle, 0.f));
            }
            DetectAttackHits();
        })
        .SetEase(ETweenEase::OutBack)
        .OnComplete([this, OvershootAngle]() {
            UTween::Value(this, OvershootAngle, SwingToAngle, 0.035f, [this](float Angle) {
                if (SwordPivot)
                {
                    SwordPivot->SetRelativeRotation(FRotator(0.f, Angle, 0.f));
                }
            })
            .SetEase(ETweenEase::InOutQuad)
            .OnComplete([this]() {
                bIsAttacking = false;
                CurrentRestAngle = SwingToAngle;
                if (SwordPivot)
                {
                    SwordPivot->SetRelativeRotation(FRotator(0.f, CurrentRestAngle, 0.f));
                }
            });
        });
    });
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

                Enemy->TakeHit(AttackDamage, HitDir, bIsPolished);

                if (bIsPolished)
                {
                    TriggerHitstop(0.055f);
                    AddCameraTrauma(0.35f);
                    TriggerCameraPunchZoom(790.f, 0.05f, 0.20f);
                }
            }
        }
    }
}

void AGameFeelPlayerCharacter::TriggerHitstop(float Duration)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UGameplayStatics::SetGlobalTimeDilation(World, 0.001f);

    UTween::Value(this, 0.f, 1.f, Duration, [](float) {})
        .SetTimeMode(ETweenTimeMode::RealTime)
        .OnComplete([WorldWeak = TWeakObjectPtr<UWorld>(World)]() {
            if (WorldWeak.IsValid())
            {
                UGameplayStatics::SetGlobalTimeDilation(WorldWeak.Get(), 1.0f);
            }
        });
}

void AGameFeelPlayerCharacter::TogglePolishMode()
{
    SetPolishMode(!bIsPolished);
}

void AGameFeelPlayerCharacter::SetPolishMode(bool bEnable)
{
    bIsPolished = bEnable;

    if (!bIsPolished)
    {
        CameraTrauma = 0.f;
        if (TopDownCamera)
        {
            TopDownCamera->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
        }
        if (CameraBoom)
        {
            CameraBoom->SocketOffset = FVector::ZeroVector;
            CameraBoom->TargetArmLength = DefaultBoomLength;
        }
    }
}

void AGameFeelPlayerCharacter::TakePlayerDamage(float Damage)
{
    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
}