#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TweenHandle.h"
#include "GameFeelHUDWidget.generated.h"

UCLASS()
class UTWEENPROJECT_API UGameFeelHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UGameFeelHUDWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual int32 NativePaint(
        const FPaintArgs& Args,
        const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements,
        int32 LayerId,
        const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled) const override;

    UFUNCTION(BlueprintCallable, Category = "GameFeel|UI")
    void SetPolishStatus(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "GameFeel|UI")
    void SetCombo(int32 NewCombo);

    void SpawnCombatFeedbackText(float DamageAmount, const FVector& WorldLocation, int32 ComboStep, bool bIsPolishedHit);

    UFUNCTION(BlueprintImplementableEvent, Category = "GameFeel|UI")
    void OnPolishStatusChanged(bool bEnabled);

    UFUNCTION(BlueprintImplementableEvent, Category = "GameFeel|UI")
    void OnComboUpdated(int32 NewCombo);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameFeel|UI")
    bool bPolishEnabled = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameFeel|UI")
    int32 CurrentCombo = 0;

    float ComboDisplayScale = 1.0f;
    float ComboAlpha = 0.0f;
    float ComboFadeTimer = 0.0f;
    float PolishBadgePulse = 1.0f;

private:
    struct FFloatingCombatEntry
    {
        FVector WorldOrigin = FVector::ZeroVector;
        FVector2D ScreenOffset = FVector2D::ZeroVector;
        FVector2D ScreenVelocity = FVector2D::ZeroVector;
        FString DamageText;
        FLinearColor PrimaryColor = FLinearColor::White;
        float Elapsed = 0.0f;
        float Duration = 0.85f;
        float Scale = 1.0f;
        int32 BaseFontSize = 22;
        bool bPolished = true;
    };

    struct FEnemyLifeBarState
    {
        FVector LastWorldLocation = FVector::ZeroVector;
        float LastKnownHealthPercent = 1.0f;
        float DisplayedPercent = 1.0f;
        float TrailingPercent = 1.0f;
        float TrailingDelayTimer = 0.0f;
        float Alpha = 0.0f;
        float Scale = 1.0f;
        float ShakeTimer = 0.0f;
        float DeathHoldTimer = 0.0f;
    };

    TArray<FFloatingCombatEntry> ActiveFloatingEntries;
    TMap<TWeakObjectPtr<class AGameFeelEnemyCharacter>, FEnemyLifeBarState> EnemyLifeBars;
    FTweenHandle PolishBadgeTweenHandle;
};
