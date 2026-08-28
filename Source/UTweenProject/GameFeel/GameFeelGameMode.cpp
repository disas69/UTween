#include "GameFeelGameMode.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#include "GameFeelEnemyCharacter.h"
#include "GameFeelPlayerCharacter.h"
#include "GameFeelPlayerController.h"

AGameFeelGameMode::AGameFeelGameMode()
{
    DefaultPawnClass = AGameFeelPlayerCharacter::StaticClass();
    PlayerControllerClass = AGameFeelPlayerController::StaticClass();
    EnemyClass = AGameFeelEnemyCharacter::StaticClass();

    static ConstructorHelpers::FClassFinder<AGameFeelPlayerCharacter> PlayerBP(TEXT("/Game/GameFeel/Blueprints/BP_GameFeelPlayer.BP_GameFeelPlayer_C"));
    if (PlayerBP.Succeeded())
    {
        DefaultPawnClass = PlayerBP.Class;
    }

    static ConstructorHelpers::FClassFinder<AGameFeelEnemyCharacter> EnemyBP(TEXT("/Game/GameFeel/Blueprints/BP_GameFeelEnemy.BP_GameFeelEnemy_C"));
    if (EnemyBP.Succeeded())
    {
        EnemyClass = EnemyBP.Class;
    }
}

void AGameFeelGameMode::BeginPlay()
{
    Super::BeginPlay();

    SpawnEnemyRing();
}

void AGameFeelGameMode::SpawnEnemyRing()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Check if enemies already exist in the level
    TArray<AActor*> ExistingEnemies;
    UGameplayStatics::GetAllActorsOfClass(World, AGameFeelEnemyCharacter::StaticClass(), ExistingEnemies);
    if (ExistingEnemies.Num() > 0)
    {
        return;
    }

    const TSubclassOf<AGameFeelEnemyCharacter> ClassToSpawn = EnemyClass ? EnemyClass : TSubclassOf<AGameFeelEnemyCharacter>(AGameFeelEnemyCharacter::StaticClass());
    const FVector Center = FVector(0.f, 0.f, 65.f);

    for (int32 i = 0; i < EnemyCount; ++i)
    {
        const float AngleRad = (2.f * PI / static_cast<float>(EnemyCount)) * static_cast<float>(i);
        const FVector SpawnLoc = Center + FVector(FMath::Cos(AngleRad) * RingRadius, FMath::Sin(AngleRad) * RingRadius, 0.f);
        const FRotator SpawnRot = (Center - SpawnLoc).Rotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        World->SpawnActor<AGameFeelEnemyCharacter>(ClassToSpawn, SpawnLoc, SpawnRot, SpawnParams);
    }
}