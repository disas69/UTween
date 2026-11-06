#include "GameFeelPlayerController.h"

#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

#include "Blueprint/UserWidget.h"
#include "GameFeelHUDWidget.h"

AGameFeelPlayerController::AGameFeelPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    static ConstructorHelpers::FClassFinder<UGameFeelHUDWidget> HUDFinder(TEXT("/Game/GameFeel/Blueprints/WBP_GameFeelHUD.WBP_GameFeelHUD_C"));
    if (HUDFinder.Succeeded())
    {
        HUDWidgetClass = HUDFinder.Class;
    }
}

void AGameFeelPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    if (!HUDWidgetClass)
    {
        HUDWidgetClass = UGameFeelHUDWidget::StaticClass();
    }

    if (HUDWidgetClass && IsLocalController())
    {
        HUDWidgetInstance = CreateWidget<UGameFeelHUDWidget>(this, HUDWidgetClass);
        if (HUDWidgetInstance)
        {
            HUDWidgetInstance->AddToViewport(10);
            HUDWidgetInstance->SetPolishStatus(true);
            HUDWidgetInstance->SetCombo(0);
            HUDWidgetInstance->OnPolishStatusChanged(true);
            HUDWidgetInstance->OnComboUpdated(0);
        }
    }
}

void AGameFeelPlayerController::NotifyPolishChanged(bool bEnabled)
{
    if (HUDWidgetInstance)
    {
        HUDWidgetInstance->SetPolishStatus(bEnabled);
        HUDWidgetInstance->OnPolishStatusChanged(bEnabled);
    }
}

void AGameFeelPlayerController::NotifyComboChanged(int32 Combo)
{
    if (HUDWidgetInstance)
    {
        HUDWidgetInstance->SetCombo(Combo);
        HUDWidgetInstance->OnComboUpdated(Combo);
    }
}