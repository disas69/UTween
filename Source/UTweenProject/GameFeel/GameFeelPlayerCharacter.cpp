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
#include "GameFeelPlayerController.h"
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

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CapsuleFinder(TEXT("/Game/GameFeel/Meshes/SM_GameFeel_Capsule.SM_GameFeel_Capsule"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SwordMeshFinder(TEXT("/Game/GameFeel/Meshes/SM_GameFeel_BroadSword.SM_GameFeel_BroadSword"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlayerMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_Player.MI_GameFeel_Player"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> HandMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_PlayerHand.MI_GameFeel_PlayerHand"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SwordMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_Sword.MI_GameFeel_Sword"));

    BodyCylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyCylinder"));
    BodyCylinder->SetupAttachment(VisualRoot);
    BodyCylinder->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    BodyCylinder->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
    BodyCylinder->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CapsuleFinder.Succeeded())
    {
        BodyCylinder->SetStaticMesh(CapsuleFinder.Object);
    }
    else if (CylinderFinder.Succeeded())
    {
        BodyCylinder->SetStaticMesh(CylinderFinder.Object);
        BodyCylinder->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.6f));
    }
    if (PlayerMatFinder.Succeeded()) BodyCylinder->SetMaterial(0, PlayerMatFinder.Object);

    TopCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopCapSphere"));
    TopCapSphere->SetupAttachment(VisualRoot);
    TopCapSphere->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
    TopCapSphere->SetRelativeScale3D(FVector(0.001f, 0.001f, 0.001f));
    TopCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    TopCapSphere->SetVisibility(false);
    TopCapSphere->SetHiddenInGame(true);
    if (SphereFinder.Succeeded()) TopCapSphere->SetStaticMesh(SphereFinder.Object);
    if (PlayerMatFinder.Succeeded()) TopCapSphere->SetMaterial(0, PlayerMatFinder.Object);

    BottomCapSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomCapSphere"));
    BottomCapSphere->SetupAttachment(VisualRoot);
    BottomCapSphere->SetRelativeLocation(FVector(0.f, 0.f, -30.f));
    BottomCapSphere->SetRelativeScale3D(FVector(0.001f, 0.001f, 0.001f));
    BottomCapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BottomCapSphere->SetVisibility(false);
    BottomCapSphere->SetHiddenInGame(true);
    if (SphereFinder.Succeeded()) BottomCapSphere->SetStaticMesh(SphereFinder.Object);
    if (PlayerMatFinder.Succeeded()) BottomCapSphere->SetMaterial(0, PlayerMatFinder.Object);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->SetUsingAbsoluteRotation(true);
    CameraBoom->TargetArmLength = DefaultBoomLength;
    CameraBoom->SetRelativeRotation(FRotator(-48.f, 0.f, 0.f));
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritYaw = false;
    CameraBoom->bInheritRoll = false;

    TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    TopDownCamera->bUsePawnControlRotation = false;
    TopDownCamera->FieldOfView = 45.f;

    SwordPivot = CreateDefaultSubobject<USceneComponent>(TEXT("SwordPivot"));
    SwordPivot->SetupAttachment(RootComponent);
    SwordPivot->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

    NoseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NoseMesh"));
    NoseMesh->SetupAttachment(SwordPivot);
    NoseMesh->SetRelativeLocation(FVector(36.f, 0.f, 10.f));
    NoseMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));
    NoseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereFinder.Succeeded()) NoseMesh->SetStaticMesh(SphereFinder.Object);
    if (HandMatFinder.Succeeded())
    {
        NoseMesh->SetMaterial(0, HandMatFinder.Object);
    }
    else if (PlayerMatFinder.Succeeded())
    {
        NoseMesh->SetMaterial(0, PlayerMatFinder.Object);
    }

    SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
    SwordMesh->SetupAttachment(SwordPivot);
    SwordMesh->SetRelativeLocation(FVector(76.f, 0.f, 10.f));
    SwordMesh->SetRelativeScale3D(FVector(0.74f, 0.42f, 0.08f));
    SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SwordMeshFinder.Succeeded())
    {
        SwordMesh->SetStaticMesh(SwordMeshFinder.Object);
    }
    else if (CubeFinder.Succeeded())
    {
        SwordMesh->SetStaticMesh(CubeFinder.Object);
    }
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

    bIsUsingGamepadAim = true;
    LastMousePosition = FVector2D::ZeroVector;
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
            SlashArcInstance->TriggerSlash(0.85f, FLinearColor(9.5f, 7.8f, 5.2f, 1.0f));
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

    if (bIsPolished && VisualRoot && !bIsAttacking)
    {
        const FVector Velocity = GetVelocity();
        const float Speed2D = Velocity.Size2D();
        if (Speed2D > 20.f)
        {
            LocomotionTimer += DeltaTime * (Speed2D * 0.024f);
            const float BobZ = FMath::Sin(LocomotionTimer * 2.2f) * 3.5f;
            VisualRoot->SetRelativeLocation(FVector(0.f, 0.f, BobZ));

            const FVector LocalVel = GetActorTransform().InverseTransformVector(Velocity);
            const float RollTilt = FMath::Clamp(-LocalVel.Y * 0.022f, -7.5f, 7.5f);
            const float PitchTilt = FMath::Clamp(LocalVel.X * 0.016f, -5.5f, 5.5f);
            VisualRoot->SetRelativeRotation(FRotator(PitchTilt, 0.f, RollTilt));
        }
        else
        {
            LocomotionTimer = 0.f;
            VisualRoot->SetRelativeLocation(FMath::VInterpTo(VisualRoot->GetRelativeLocation(), FVector::ZeroVector, DeltaTime, 10.f));
            VisualRoot->SetRelativeRotation(FMath::RInterpTo(VisualRoot->GetRelativeRotation(), FRotator::ZeroRotator, DeltaTime, 10.f));
        }
    }

    if (CurrentCombo > 0)
    {
        ComboTimer += DeltaTime;
        if (ComboTimer > 1.35f)
        {
            CurrentCombo = 0;
            ComboTimer = 0.f;
            if (AGameFeelPlayerController* PC = Cast<AGameFeelPlayerController>(GetController()))
            {
                PC->NotifyComboChanged(CurrentCombo);
            }
        }
    }

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

        if (!bIsUsingGamepadAim)
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
    if (AimVec.SizeSquared() > 0.04f)
    {
        bIsUsingGamepadAim = true;
        const FVector AimDir = FVector(AimVec.Y, AimVec.X, 0.f).GetSafeNormal();
        if (!AimDir.IsNearlyZero())
        {
            SetActorRotation(FRotator(0.f, AimDir.Rotation().Yaw, 0.f));
        }
    }
}

void AGameFeelPlayerCharacter::EnhancedAimCompleted(const FInputActionValue& Value)
{
    // Sticky aim retains last aim direction when stick is released
}

void AGameFeelPlayerCharacter::AimAtCursor()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    if (GetWorld() && GetWorld()->GetTimeSeconds() < 0.6f)
    {
        return;
    }

    float MouseX, MouseY;
    if (PC->GetMousePosition(MouseX, MouseY))
    {
        const FVector2D CurrentMousePos(MouseX, MouseY);
        if (LastMousePosition.IsZero())
        {
            LastMousePosition = CurrentMousePos;
        }
        else if ((CurrentMousePos - LastMousePosition).SizeSquared() > 4.0f)
        {
            bIsUsingGamepadAim = false;
            LastMousePosition = CurrentMousePos;
        }
    }

    if (bIsUsingGamepadAim)
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
        TriggerCameraPunchZoom(DefaultBoomLength - 110.f, 0.06f, 0.25f);
    }
}

void AGameFeelPlayerCharacter::PerformAttack()
{
    if (bIsAttacking)
    {
        if (UTweenSubsystem* TweenSys = UTweenSubsystem::Get(this))
        {
            TweenSys->KillByTag(FTweenTag(TEXT("PlayerAttack")));
        }

        const float CurrentAngle = SwordPivot ? SwordPivot->GetRelativeRotation().Yaw : CurrentRestAngle;
        SwingFromAngle = CurrentAngle;
        SwingToAngle = (CurrentAngle >= 0.f) ? -RestStanceAngle : RestStanceAngle;
    }
    else
    {
        SwingFromAngle = CurrentRestAngle;
        SwingToAngle = -CurrentRestAngle;
    }

    bIsAttacking = true;
    AttackTimer = 0.f;
    HitActorsThisSwing.Empty();

    if (bIsPolished)
    {
        ExecutePolishedAttack();
    }
}

void AGameFeelPlayerCharacter::ExecutePolishedAttack()
{
    const float WindupSign = (SwingFromAngle > 0.f) ? 1.f : -1.f;
    const int32 ComboStep = CurrentCombo % 3;

    FLinearColor SlashColor;
    if (ComboStep == 0)
    {
        SlashColor = FLinearColor(9.5f, 7.8f, 5.2f, 1.0f);
        if (!bIsUsingGamepadAim)
        {
            LaunchCharacter(GetActorForwardVector() * 190.f, false, false);
        }
        AddCameraTrauma(0.12f);
    }
    else if (ComboStep == 1)
    {
        SlashColor = FLinearColor(10.5f, 8.2f, 5.0f, 1.0f);
        LaunchCharacter(GetActorForwardVector() * 220.f + FVector(0.f, 0.f, 40.f), false, false);
        AddCameraTrauma(0.20f);

        if (VisualRoot)
        {
            const float LeanAngle = (SwingFromAngle > 0.f) ? 12.f : -12.f;
            UTween::ComponentRelativeRotation(VisualRoot, FRotator(-10.f, 0.f, LeanAngle), 0.08f)
                .SetEase(ETweenEase::OutQuad)
                .OnComplete([this]() {
                    if (VisualRoot)
                    {
                        UTween::ComponentRelativeRotation(VisualRoot, FRotator::ZeroRotator, 0.12f)
                            .SetEase(ETweenEase::OutQuad);
                    }
                });
        }
    }
    else
    {
        SlashColor = FLinearColor(13.0f, 9.5f, 4.5f, 1.0f);
        LaunchCharacter(GetActorForwardVector() * 260.f + FVector(0.f, 0.f, 90.f), false, false);
        AddCameraTrauma(0.35f);

        if (VisualRoot)
        {
            UTween::ComponentRelativeLocation(VisualRoot, FVector(0.f, 0.f, 28.f), 0.08f)
                .SetEase(ETweenEase::OutQuad)
                .OnComplete([this]() {
                    if (VisualRoot)
                    {
                        UTween::ComponentRelativeLocation(VisualRoot, FVector::ZeroVector, 0.08f)
                            .SetEase(ETweenEase::InBack);
                    }
                });
        }

        if (SwordMesh)
        {
            UTween::ComponentRelativeScale(SwordMesh, FVector(1.05f, 0.48f, 0.10f), 0.08f)
                .SetEase(ETweenEase::OutQuad)
                .OnComplete([this]() {
                    if (SwordMesh)
                    {
                        UTween::ComponentRelativeScale(SwordMesh, FVector(0.74f, 0.42f, 0.08f), 0.14f)
                            .SetEase(ETweenEase::OutElastic);
                    }
                });
        }
    }

    if (ComboStep < 2 && SwordMesh)
    {
        UTween::ComponentRelativeScale(SwordMesh, FVector(0.92f, 0.46f, 0.09f), 0.05f)
            .SetEase(ETweenEase::OutQuad)
            .OnComplete([this]() {
                if (SwordMesh)
                {
                    UTween::ComponentRelativeScale(SwordMesh, FVector(0.74f, 0.42f, 0.08f), 0.08f)
                        .SetEase(ETweenEase::OutQuad);
                }
            });
    }

    const float WindupAngle = SwingFromAngle + (WindupSign * 18.f);
    const float OvershootAngle = SwingToAngle - (WindupSign * 12.f);

    UTween::Value(this, SwingFromAngle, WindupAngle, 0.045f, [this](float Angle) {
        if (SwordPivot)
        {
            SwordPivot->SetRelativeRotation(FRotator(0.f, Angle, 0.f));
        }
    })
    .SetTag(FTweenTag(TEXT("PlayerAttack")))
    .SetEase(ETweenEase::OutQuad)
    .OnComplete([this, WindupAngle, OvershootAngle, SlashColor]() {
        if (SlashArcInstance)
        {
            SlashArcInstance->TriggerSlash(0.32f, SlashColor);
        }

        UTween::Value(this, WindupAngle, OvershootAngle, 0.12f, [this](float Angle) {
            if (SwordPivot)
            {
                SwordPivot->SetRelativeRotation(FRotator(0.f, Angle, 0.f));
            }
            DetectAttackHits();
        })
        .SetTag(FTweenTag(TEXT("PlayerAttack")))
        .SetEase(ETweenEase::OutBack)
        .OnComplete([this, OvershootAngle]() {
            UTween::Value(this, OvershootAngle, SwingToAngle, 0.035f, [this](float Angle) {
                if (SwordPivot)
                {
                    SwordPivot->SetRelativeRotation(FRotator(0.f, Angle, 0.f));
                }
            })
            .SetTag(FTweenTag(TEXT("PlayerAttack")))
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
                    CurrentCombo++;
                    ComboTimer = 0.f;
                    if (AGameFeelPlayerController* PC = Cast<AGameFeelPlayerController>(GetController()))
                    {
                        PC->NotifyComboChanged(CurrentCombo);
                    }

                    if (CurrentCombo >= 3)
                    {
                        CurrentCombo = 0;
                        TriggerComboFinishFlourish();
                    }
                    else
                    {
                        TriggerHitstop(0.08f);
                        AddCameraTrauma(0.35f);
                        TriggerCameraPunchZoom(DefaultBoomLength - 70.f, 0.05f, 0.20f);
                    }
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

void AGameFeelPlayerCharacter::TriggerComboFinishFlourish()
{
    UWorld* World = GetWorld();
    if (!World || !bIsPolished)
    {
        return;
    }

    AddCameraTrauma(0.70f);
    TriggerCameraPunchZoom(DefaultBoomLength - 150.f, 0.06f, 0.35f);

    UGameplayStatics::SetGlobalTimeDilation(World, 0.10f);

    UTween::Value(this, 0.10f, 1.0f, 0.30f, [WorldWeak = TWeakObjectPtr<UWorld>(World)](float Dilation) {
        if (WorldWeak.IsValid())
        {
            UGameplayStatics::SetGlobalTimeDilation(WorldWeak.Get(), Dilation);
        }
    })
    .SetDelay(0.12f)
    .SetTimeMode(ETweenTimeMode::RealTime)
    .SetEase(ETweenEase::OutQuad);
}

void AGameFeelPlayerCharacter::TogglePolishMode()
{
    SetPolishMode(!bIsPolished);
}

void AGameFeelPlayerCharacter::SetPolishMode(bool bEnable)
{
    bIsPolished = bEnable;

    if (AGameFeelPlayerController* PC = Cast<AGameFeelPlayerController>(GetController()))
    {
        PC->NotifyPolishChanged(bIsPolished);
    }

    if (!bIsPolished)
    {
        CurrentCombo = 0;
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