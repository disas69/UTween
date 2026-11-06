#include "GameFeelHUDWidget.h"

#include "Fonts/SlateFontInfo.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "UTween.h"

UGameFeelHUDWidget::UGameFeelHUDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(false);
}

void UGameFeelHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UGameFeelHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (CurrentCombo > 0)
    {
        ComboFadeTimer += InDeltaTime;
        if (ComboFadeTimer > 1.25f)
        {
            ComboAlpha = FMath::FInterpTo(ComboAlpha, 0.0f, InDeltaTime, 5.0f);
        }
    }
    else
    {
        ComboAlpha = FMath::FInterpTo(ComboAlpha, 0.0f, InDeltaTime, 8.0f);
    }
}

void UGameFeelHUDWidget::SetPolishStatus(bool bEnabled)
{
    bPolishEnabled = bEnabled;
    PolishBadgePulse = 1.35f;
    UTween::Value(this, 1.35f, 1.0f, 0.22f, [this](float Pulse) {
        PolishBadgePulse = Pulse;
    })
    .SetEase(ETweenEase::OutBack);
}

void UGameFeelHUDWidget::SetCombo(int32 NewCombo)
{
    CurrentCombo = NewCombo;
    if (CurrentCombo > 0)
    {
        ComboAlpha = 1.0f;
        ComboFadeTimer = 0.0f;
        ComboDisplayScale = 1.70f;
        UTween::Value(this, 1.70f, 1.0f, 0.16f, [this](float Scale) {
            ComboDisplayScale = Scale;
        })
        .SetEase(ETweenEase::OutBack);
    }
}

int32 UGameFeelHUDWidget::NativePaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
    LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    const FVector2D ViewportSize = AllottedGeometry.GetLocalSize();
    if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f)
    {
        return LayerId;
    }

    const FSlateBrush* DefaultBrush = FCoreStyle::Get().GetDefaultBrush();
    const FSlateFontInfo HeadlineFont = FCoreStyle::GetDefaultFontStyle("Bold", FMath::RoundToInt(42.0f * PolishBadgePulse));
    const FSlateFontInfo ComboFont = FCoreStyle::GetDefaultFontStyle("Bold", FMath::RoundToInt(22.0f * ComboDisplayScale));
    const FSlateFontInfo LegendFont = FCoreStyle::GetDefaultFontStyle("Regular", 11);

    // 1. Top-Center Reference Headline ("+ ATTACK FEEDBACK")
    {
        const FString HeadlineText = bPolishEnabled
            ? TEXT("+ ATTACK FEEDBACK")
            : TEXT("- NO ATTACK FEEDBACK");

        const FLinearColor HeadlineColor = bPolishEnabled
            ? FLinearColor(0.98f, 0.98f, 0.99f, 0.98f)
            : FLinearColor(0.55f, 0.55f, 0.60f, 0.85f);

        const float ApproxHalfWidth = bPolishEnabled ? 235.0f * PolishBadgePulse : 265.0f * PolishBadgePulse;
        const FVector2D HeadlinePos(ViewportSize.X * 0.5f - ApproxHalfWidth, ViewportSize.Y * 0.105f);

        // Subtle drop shadow for crisp legibility matching reference video
        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(HeadlinePos + FVector2D(2.0f, 3.0f))),
            FText::FromString(HeadlineText),
            HeadlineFont,
            ESlateDrawEffect::None,
            FLinearColor(0.0f, 0.0f, 0.0f, 0.45f));

        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(HeadlinePos)),
            FText::FromString(HeadlineText),
            HeadlineFont,
            ESlateDrawEffect::None,
            HeadlineColor);
    }

    // 2. Subtle Combo Sub-Callout under Headline
    if (ComboAlpha > 0.01f && CurrentCombo > 1)
    {
        const FString ComboStr = (CurrentCombo == 2)
            ? TEXT("COMBO x2")
            : TEXT("FINISHER!");
        const FLinearColor ComboColor = FLinearColor(1.0f, 0.84f, 0.28f, ComboAlpha * 0.92f);
        const FVector2D ComboCenterPos(ViewportSize.X * 0.5f - 68.0f, ViewportSize.Y * 0.175f);

        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(ComboCenterPos)),
            FText::FromString(ComboStr),
            ComboFont,
            ESlateDrawEffect::None,
            ComboColor);
    }

    // 3. Minimal Unobtrusive Bottom Controls Hint
    {
        const FString LegendText = TEXT("[WASD] Move    [Mouse] Aim    [LMB] Slash    [T] Toggle Feedback");
        const FVector2D LegendPos(ViewportSize.X * 0.5f - 195.0f, ViewportSize.Y - 34.0f);

        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(LegendPos)),
            FText::FromString(LegendText),
            LegendFont,
            ESlateDrawEffect::None,
            FLinearColor(0.75f, 0.76f, 0.80f, 0.45f));
    }

    return LayerId;
}