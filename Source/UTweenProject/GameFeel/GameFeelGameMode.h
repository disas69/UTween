#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFeelGameMode.generated.h"

class AGameFeelEnemyCharacter;

UCLASS()
class UTWEENPROJECT_API AGameFeelGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AGameFeelGameMode();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category = "GameFeel|GameMode")
    void SpawnEnemyRing();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Spawning")
    int32 EnemyCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Spawning")
    float RingRadius = 260.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Spawning")
    TSubclassOf<AGameFeelEnemyCharacter> EnemyClass;
};