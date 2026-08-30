#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFeelDamageNumber.generated.h"

class UTextRenderComponent;

UCLASS()
class UTWEENPROJECT_API AGameFeelDamageNumber : public AActor
{
    GENERATED_BODY()

public:
    AGameFeelDamageNumber();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "GameFeel|UI")
    void InitializeDamage(float DamageAmount, const FVector& SpawnLocation);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UTextRenderComponent> TextComponent;

protected:
    virtual void BeginPlay() override;

private:
    void FaceCamera();
};