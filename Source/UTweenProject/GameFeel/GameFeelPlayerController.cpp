#include "GameFeelPlayerController.h"

#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

AGameFeelPlayerController::AGameFeelPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCFinder(TEXT("/Game/GameFeel/Input/IMC_GameFeel.IMC_GameFeel"));
    if (IMCFinder.Succeeded())
    {
        DefaultMappingContext = IMCFinder.Object;
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
}