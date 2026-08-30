#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFeelHUDWidget.generated.h"

UCLASS()
class UTWEENPROJECT_API UGameFeelHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "GameFeel|UI")
    void OnPolishStatusChanged(bool bEnabled);

    UFUNCTION(BlueprintImplementableEvent, Category = "GameFeel|UI")
    void OnComboUpdated(int32 NewCombo);
};