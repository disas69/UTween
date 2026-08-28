#include "JuiceLabHUD.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "JuiceChestActor.h"
#include "JuiceDummyActor.h"
#include "JuiceLabWidget.h"

AJuiceLabHUD::AJuiceLabHUD()
{
    WidgetClass = UJuiceLabWidget::StaticClass();
}

void AJuiceLabHUD::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetOwningPlayerController();
    if (!PC)
    {
        return;
    }

    PC->bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    PC->SetInputMode(InputMode);

    if (WidgetClass)
    {
        ActiveWidget = CreateWidget<UJuiceLabWidget>(PC, WidgetClass);
        if (ActiveWidget)
        {
            ActiveWidget->AddToViewport(10);

            AJuiceDummyActor* Dummy = Cast<AJuiceDummyActor>(
                UGameplayStatics::GetActorOfClass(GetWorld(), AJuiceDummyActor::StaticClass()));
            AJuiceChestActor* Chest = Cast<AJuiceChestActor>(
                UGameplayStatics::GetActorOfClass(GetWorld(), AJuiceChestActor::StaticClass()));

            ActiveWidget->SetTargetActors(Dummy, Chest);
        }
    }
}
