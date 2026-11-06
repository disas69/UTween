#include "GameFeelDamageNumber.h"

#include "Camera/CameraComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "GameFeelHUDWidget.h"
#include "GameFeelPlayerCharacter.h"
#include "GameFeelPlayerController.h"
#include "UTween.h"

AGameFeelDamageNumber::AGameFeelDamageNumber()
{
    PrimaryActorTick.bCanEverTick = false;

    TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComponent"));
    RootComponent = TextComponent;

    TextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    TextComponent->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    TextComponent->SetWorldSize(38.f);
    TextComponent->SetTextRenderColor(FColor(255, 240, 100, 0));
    TextComponent->SetText(FText::GetEmpty());
    TextComponent->SetVisibility(false);
    TextComponent->SetHiddenInGame(true);
}

void AGameFeelDamageNumber::BeginPlay()
{
    Super::BeginPlay();
    if (TextComponent)
    {
        TextComponent->SetVisibility(false);
        TextComponent->SetHiddenInGame(true);
    }
}

void AGameFeelDamageNumber::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AGameFeelDamageNumber::FaceCamera()
{
}

void AGameFeelDamageNumber::InitializeDamage(float DamageAmount, const FVector& SpawnLocation)
{
    if (TextComponent)
    {
        TextComponent->SetVisibility(false);
        TextComponent->SetHiddenInGame(true);
    }

    bool bPolished = true;
    int32 ComboStep = 1;

    if (AGameFeelPlayerCharacter* Player = Cast<AGameFeelPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        bPolished = Player->IsPolishEnabled();
    }

    if (AGameFeelPlayerController* PC = Cast<AGameFeelPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        if (PC->HUDWidgetInstance)
        {
            if (DamageAmount >= 45.f)
            {
                ComboStep = 3;
            }
            else if (DamageAmount >= 34.f)
            {
                ComboStep = 2;
            }
            PC->HUDWidgetInstance->SpawnCombatFeedbackText(DamageAmount, SpawnLocation, ComboStep, bPolished);
        }
    }

    SetLifeSpan(0.05f);
}
