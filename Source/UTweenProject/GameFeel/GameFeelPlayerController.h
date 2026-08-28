#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFeelPlayerController.generated.h"

UCLASS()
class UTWEENPROJECT_API AGameFeelPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AGameFeelPlayerController();

protected:
    virtual void BeginPlay() override;
};