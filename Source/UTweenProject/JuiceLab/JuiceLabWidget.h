#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JuiceLabWidget.generated.h"

class UCanvasPanel;
class UProgressBar;
class UTextBlock;
class UButton;
class AJuiceDummyActor;
class AJuiceChestActor;

UCLASS(Blueprintable, BlueprintType)
class UTWEENPROJECT_API UJuiceLabWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UJuiceLabWidget(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|UI")
    void SetTargetActors(AJuiceDummyActor* InDummy, AJuiceChestActor* InChest);

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|UI")
    void ApplyDamage(float Damage, bool bIsCritical);

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|UI")
    void ResetHealth();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|UI")
    void SpawnDamageNumber(float Damage, const FVector& WorldLocation, bool bIsCritical, int32 ComboCount = 0);

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|UI")
    void UpdateComboText(int32 ComboCount);

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|UI")
    void SetTimeDilation(float NewTimeDilation);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

private:
    void BuildProceduralUI();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Actions")
    void HandleLightAttack();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Actions")
    void HandleHeavyAttack();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Actions")
    void HandleCritAttack();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Actions")
    void HandleToggleChest();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Actions")
    void HandleReset();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Actions")
    void HandleNormalSpeed();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Actions")
    void HandleSlomoSpeed();

    UFUNCTION(BlueprintCallable, Category = "JuiceLab|Actions")
    void HandlePauseSpeed();

    UPROPERTY(Transient)
    TObjectPtr<AJuiceDummyActor> DummyActor;

    UPROPERTY(Transient)
    TObjectPtr<AJuiceChestActor> ChestActor;

    UPROPERTY()
    TObjectPtr<UCanvasPanel> RootCanvas;

    UPROPERTY()
    TObjectPtr<UCanvasPanel> DamageNumberCanvas;

    UPROPERTY()
    TObjectPtr<UWidget> HealthContainer;

    UPROPERTY()
    TObjectPtr<UProgressBar> PrimaryHealthBar;

    UPROPERTY()
    TObjectPtr<UProgressBar> GhostHealthBar;

    UPROPERTY()
    TObjectPtr<UTextBlock> HealthText;

    UPROPERTY()
    TObjectPtr<UTextBlock> ComboText;

    UPROPERTY()
    TObjectPtr<UTextBlock> TimeDilationLabel;

    UPROPERTY()
    TObjectPtr<UButton> BtnLight;

    UPROPERTY()
    TObjectPtr<UButton> BtnHeavy;

    UPROPERTY()
    TObjectPtr<UButton> BtnCrit;

    UPROPERTY()
    TObjectPtr<UButton> BtnChest;

    UPROPERTY()
    TObjectPtr<UButton> BtnReset;

    UPROPERTY()
    TObjectPtr<UButton> BtnSpeed1;

    UPROPERTY()
    TObjectPtr<UButton> BtnSpeed01;

    UPROPERTY()
    TObjectPtr<UButton> BtnSpeed0;

    float MaxHealth = 1000.f;
    float CurrentHealth = 1000.f;
    float CurrentHealthPercent = 1.0f;
    float GhostHealthPercent = 1.0f;
};
