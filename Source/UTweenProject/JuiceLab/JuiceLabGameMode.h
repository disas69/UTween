#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "JuiceLabGameMode.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UTWEENPROJECT_API AJuiceLabGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AJuiceLabGameMode();

protected:
    virtual void BeginPlay() override;

private:
    void EnsureScenePopulated();
};
