#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameFeelPlayerController.generated.h"

class UInputMappingContext;
class UGameFeelHUDWidget;

UCLASS()
class UTWEENPROJECT_API AGameFeelPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AGameFeelPlayerController();

    UFUNCTION(BlueprintCallable, Category = "GameFeel|UI")
    void NotifyPolishChanged(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "GameFeel|UI")
    void NotifyComboChanged(int32 Combo);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|UI")
    TSubclassOf<UGameFeelHUDWidget> HUDWidgetClass;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "GameFeel|UI")
    TObjectPtr<UGameFeelHUDWidget> HUDWidgetInstance;

protected:
    virtual void BeginPlay() override;
};