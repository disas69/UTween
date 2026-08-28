#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "JuiceLabHUD.generated.h"

class UJuiceLabWidget;
class AJuiceDummyActor;
class AJuiceChestActor;

UCLASS(Blueprintable, BlueprintType)
class UTWEENPROJECT_API AJuiceLabHUD : public AHUD
{
    GENERATED_BODY()

public:
    AJuiceLabHUD();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JuiceLab|UI")
    TSubclassOf<UJuiceLabWidget> WidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "JuiceLab|UI")
    TObjectPtr<UJuiceLabWidget> ActiveWidget;

protected:
    virtual void BeginPlay() override;
};
