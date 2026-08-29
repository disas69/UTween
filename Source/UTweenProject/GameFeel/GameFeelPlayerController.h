#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFeelPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class UTWEENPROJECT_API AGameFeelPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AGameFeelPlayerController();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

protected:
    virtual void BeginPlay() override;
};