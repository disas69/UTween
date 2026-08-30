#include "GameFeelDamageNumber.h"

#include "Camera/CameraComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "UTween.h"

AGameFeelDamageNumber::AGameFeelDamageNumber()
{
    PrimaryActorTick.bCanEverTick = true;

    TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComponent"));
    RootComponent = TextComponent;

    TextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    TextComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    TextComponent->SetWorldSize(38.f);
    TextComponent->SetTextRenderColor(FColor(255, 240, 100, 255));
    TextComponent->SetText(FText::FromString(TEXT("30")));

    SetActorScale3D(FVector(0.2f, 0.2f, 0.2f));
}

void AGameFeelDamageNumber::BeginPlay()
{
    Super::BeginPlay();
    FaceCamera();
}

void AGameFeelDamageNumber::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    FaceCamera();
}

void AGameFeelDamageNumber::FaceCamera()
{
    APlayerCameraManager* CamManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (CamManager)
    {
        const FVector CamLoc = CamManager->GetCameraLocation();
        const FVector Dir = (CamLoc - GetActorLocation()).GetSafeNormal();
        SetActorRotation(Dir.Rotation());
    }
}

void AGameFeelDamageNumber::InitializeDamage(float DamageAmount, const FVector& SpawnLocation)
{
    const int32 IntDmg = FMath::RoundToInt(DamageAmount);
    if (TextComponent)
    {
        TextComponent->SetText(FText::FromString(FString::Printf(TEXT("%d"), IntDmg)));
    }

    const float RandomAngle = FMath::RandRange(-45.f, 45.f);
    const FVector LateralOffset = FRotator(0.f, RandomAngle, 0.f).RotateVector(FVector(FMath::RandRange(20.f, 45.f), 0.f, 0.f));
    const FVector StartLoc = SpawnLocation + FVector(0.f, 0.f, 25.f);
    const FVector EndLoc = StartLoc + LateralOffset + FVector(0.f, 0.f, 75.f);

    SetActorLocation(StartLoc);
    SetActorScale3D(FVector(0.25f, 0.25f, 0.25f));

    // 1. Pop Scale
    UTween::ActorScale(this, FVector(1.35f, 1.35f, 1.35f), 0.08f)
        .SetEase(ETweenEase::OutBack)
        .OnComplete([this]() {
            UTween::ActorScale(this, FVector(1.0f, 1.0f, 1.0f), 0.12f)
                .SetEase(ETweenEase::OutQuad);
        });

    // 2. Rise Location
    UTween::ActorLocation(this, EndLoc, 0.55f)
        .SetEase(ETweenEase::OutQuad);

    // 3. Fade Out and Destroy
    UTween::Value(this, 1.0f, 0.0f, 0.20f, [this](float Alpha) {
        if (TextComponent)
        {
            FColor Col = TextComponent->TextRenderColor;
            Col.A = FMath::Clamp(FMath::RoundToInt(Alpha * 255.f), 0, 255);
            TextComponent->SetTextRenderColor(Col);
        }
    })
    .SetDelay(0.35f)
    .SetEase(ETweenEase::InQuad)
    .OnComplete([this]() {
        Destroy();
    });
}