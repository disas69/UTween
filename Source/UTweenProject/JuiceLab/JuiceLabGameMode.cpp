#include "JuiceLabGameMode.h"

#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#include "JuiceChestActor.h"
#include "JuiceDummyActor.h"
#include "JuiceLabHUD.h"
#include "JuiceLabWidget.h"

AJuiceLabGameMode::AJuiceLabGameMode()
{
    HUDClass = AJuiceLabHUD::StaticClass();
    DefaultPawnClass = ASpectatorPawn::StaticClass();
}

void AJuiceLabGameMode::BeginPlay()
{
    Super::BeginPlay();

    EnsureScenePopulated();
}

void AJuiceLabGameMode::EnsureScenePopulated()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AJuiceDummyActor* Dummy = Cast<AJuiceDummyActor>(
        UGameplayStatics::GetActorOfClass(World, AJuiceDummyActor::StaticClass()));
    if (!Dummy)
    {
        const FTransform DummyTransform(FRotator(0.f, 180.f, 0.f), FVector(350.f, -120.f, 0.f));
        Dummy = World->SpawnActor<AJuiceDummyActor>(AJuiceDummyActor::StaticClass(), DummyTransform);
    }

    AJuiceChestActor* Chest = Cast<AJuiceChestActor>(
        UGameplayStatics::GetActorOfClass(World, AJuiceChestActor::StaticClass()));
    if (!Chest)
    {
        const FTransform ChestTransform(FRotator(0.f, 180.f, 0.f), FVector(350.f, 120.f, 0.f));
        Chest = World->SpawnActor<AJuiceChestActor>(AJuiceChestActor::StaticClass(), ChestTransform);
    }

    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        PC->SetControlRotation(FRotator(-8.f, 0.f, 0.f));
        if (APawn* Pawn = PC->GetPawn())
        {
            Pawn->SetActorLocation(FVector(0.f, 0.f, 130.f));
        }

        if (AJuiceLabHUD* HUD = Cast<AJuiceLabHUD>(PC->GetHUD()))
        {
            if (HUD->ActiveWidget)
            {
                HUD->ActiveWidget->SetTargetActors(Dummy, Chest);
            }
        }
    }
}
