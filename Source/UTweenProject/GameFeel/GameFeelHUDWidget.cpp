#include "GameFeelHUDWidget.h"

#include "Brushes/SlateColorBrush.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Fonts/FontMeasure.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

#include "GameFeelEnemyCharacter.h"
#include "UTween.h"

namespace
{
    void DrawOutlinedText(
        FSlateWindowElementList& OutDrawElements,
        int32& LayerId,
        const FGeometry& AllottedGeometry,
        const FString& Text,
        const FSlateFontInfo& Font,
        const FVector2D& CenterPos,
        const FLinearColor& FillColor,
        float OutlineAlpha,
        bool bDrawOutline)
    {
        FSlateFontInfo StyledFont = Font;
        if (bDrawOutline && OutlineAlpha > 0.01f)
        {
            StyledFont.OutlineSettings.OutlineSize = 2;
            StyledFont.OutlineSettings.OutlineColor = FLinearColor(0.01f, 0.01f, 0.03f, OutlineAlpha * 0.96f);
        }

        FVector2D TextSize(Text.Len() * StyledFont.Size * 0.58f, StyledFont.Size * 1.2f);
        if (FSlateApplication::IsInitialized() && FSlateApplication::Get().GetRenderer())
        {
            TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Text, StyledFont);
        }

        const FVector2D TopLeft = CenterPos - (TextSize * 0.5f);

        if (bDrawOutline && OutlineAlpha > 0.01f)
        {
            const FLinearColor ShadowColor(0.01f, 0.01f, 0.02f, OutlineAlpha * 0.75f);
            FSlateDrawElement::MakeText(
                OutDrawElements,
                LayerId++,
                AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(TopLeft + FVector2D(0.0f, 2.0f))),
                FText::FromString(Text),
                StyledFont,
                ESlateDrawEffect::None,
                ShadowColor);
        }

        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.ToPaintGeometry(FSlateLayoutTransform(TopLeft)),
            FText::FromString(Text),
            StyledFont,
            ESlateDrawEffect::None,
            FillColor);
    }
}

UGameFeelHUDWidget::UGameFeelHUDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(false);
}

void UGameFeelHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    ActiveFloatingEntries.Empty();
    EnemyLifeBars.Empty();
}

void UGameFeelHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundEnemies;
        UGameplayStatics::GetAllActorsOfClass(World, AGameFeelEnemyCharacter::StaticClass(), FoundEnemies);
        for (AActor* Actor : FoundEnemies)
        {
            if (AGameFeelEnemyCharacter* Enemy = Cast<AGameFeelEnemyCharacter>(Actor))
            {
                FEnemyLifeBarState& State = EnemyLifeBars.FindOrAdd(Enemy);
                if (!Enemy->IsHidden())
                {
                    State.LastWorldLocation = Enemy->GetActorLocation();
                }

                const float TargetPct = FMath::Clamp(Enemy->GetHealthPercent(), 0.0f, 1.0f);

                // Respawn reset detection
                if (TargetPct > State.LastKnownHealthPercent + 0.1f)
                {
                    State.LastKnownHealthPercent = TargetPct;
                    State.DisplayedPercent = TargetPct;
                    State.TrailingPercent = TargetPct;
                    State.TrailingDelayTimer = 0.0f;
                    State.Alpha = 0.0f;
                    State.Scale = 1.0f;
                    State.ShakeTimer = 0.0f;
                    State.DeathHoldTimer = 0.0f;
                    continue;
                }

                // New damage taken detection
                if (TargetPct < State.LastKnownHealthPercent - 0.001f)
                {
                    State.LastKnownHealthPercent = TargetPct;
                    State.Alpha = 1.0f;
                    State.Scale = 1.32f;
                    State.ShakeTimer = 0.16f;
                    State.TrailingDelayTimer = 0.14f;
                    if (TargetPct <= 0.001f)
                    {
                        State.DeathHoldTimer = 0.36f;
                    }
                }

                State.DisplayedPercent = FMath::FInterpTo(State.DisplayedPercent, TargetPct, InDeltaTime, 24.0f);
                State.Scale = FMath::FInterpTo(State.Scale, 1.0f, InDeltaTime, 14.0f);
                State.ShakeTimer = FMath::Max(0.0f, State.ShakeTimer - InDeltaTime);

                if (State.TrailingDelayTimer > 0.0f)
                {
                    State.TrailingDelayTimer -= InDeltaTime;
                }
                else
                {
                    State.TrailingPercent = FMath::FInterpTo(State.TrailingPercent, TargetPct, InDeltaTime, 6.5f);
                }

                if (TargetPct <= 0.001f || Enemy->IsHidden())
                {
                    if (State.DeathHoldTimer > 0.0f)
                    {
                        State.DeathHoldTimer -= InDeltaTime;
                    }
                    else
                    {
                        State.Alpha = FMath::FInterpTo(State.Alpha, 0.0f, InDeltaTime, 9.0f);
                    }
                }
                else if (TargetPct < 0.999f)
                {
                    State.Alpha = 1.0f;
                }
            }
        }
    }

    for (int32 i = ActiveFloatingEntries.Num() - 1; i >= 0; --i)
    {
        FFloatingCombatEntry& Entry = ActiveFloatingEntries[i];
        Entry.Elapsed += InDeltaTime;
        if (Entry.Elapsed >= Entry.Duration)
        {
            ActiveFloatingEntries.RemoveAt(i);
            continue;
        }

        Entry.ScreenOffset += Entry.ScreenVelocity * InDeltaTime;
        if (Entry.bPolished)
        {
            Entry.ScreenVelocity.Y += 230.0f * InDeltaTime;
            Entry.ScreenVelocity.X *= FMath::Max(0.0f, 1.0f - InDeltaTime * 3.5f);

            const float NormTime = Entry.Elapsed / Entry.Duration;
            if (NormTime < 0.14f)
            {
                const float PopT = NormTime / 0.14f;
                Entry.Scale = FMath::Lerp(0.40f, 1.36f, FMath::Sin(PopT * PI * 0.5f));
            }
            else if (NormTime < 0.30f)
            {
                const float SettleT = (NormTime - 0.14f) / 0.16f;
                Entry.Scale = FMath::Lerp(1.36f, 1.0f, SettleT);
            }
            else
            {
                Entry.Scale = 1.0f;
            }
        }
        else
        {
            Entry.Scale = 1.0f;
        }
    }
}

void UGameFeelHUDWidget::SetPolishStatus(bool bEnabled)
{
    bPolishEnabled = bEnabled;
    PolishBadgePulse = 1.32f;
    PolishBadgeTweenHandle.Kill();

    TWeakObjectPtr<UGameFeelHUDWidget> WeakThis(this);
    PolishBadgeTweenHandle = UTween::Value(this, 1.32f, 1.0f, 0.24f, [WeakThis](float Pulse) {
        if (UGameFeelHUDWidget* Self = WeakThis.Get())
        {
            Self->PolishBadgePulse = Pulse;
        }
    })
    .SetTimeMode(ETweenTimeMode::RealTime)
    .SetEase(ETweenEase::OutBack);
}

void UGameFeelHUDWidget::SetCombo(int32 NewCombo)
{
    CurrentCombo = NewCombo;
}

void UGameFeelHUDWidget::SpawnCombatFeedbackText(float DamageAmount, const FVector& WorldLocation, int32 ComboStep, bool bIsPolishedHit)
{
    FFloatingCombatEntry Entry;
    Entry.WorldOrigin = WorldLocation + FVector(0.0f, 0.0f, 95.0f);
    Entry.bPolished = bIsPolishedHit;

    const int32 RoundedDmg = FMath::RoundToInt(DamageAmount);
    Entry.DamageText = FString::Printf(TEXT("-%d"), RoundedDmg);

    Entry.PrimaryColor = FLinearColor::White;

    if (!bIsPolishedHit)
    {
        Entry.BaseFontSize = 16;
        Entry.Duration = 0.55f;
        Entry.ScreenVelocity = FVector2D(0.0f, -60.0f);
        ActiveFloatingEntries.Add(Entry);
        return;
    }

    const float RandDirX = FMath::RandBool() ? FMath::RandRange(22.0f, 52.0f) : FMath::RandRange(-52.0f, -22.0f);
    Entry.ScreenVelocity = FVector2D(RandDirX, FMath::RandRange(-135.0f, -110.0f));
    Entry.Duration = 0.88f;

    if (ComboStep >= 3)
    {
        Entry.BaseFontSize = 26;
        Entry.Duration = 0.98f;
    }
    else if (ComboStep == 2)
    {
        Entry.BaseFontSize = 23;
    }
    else
    {
        Entry.BaseFontSize = 20;
    }

    ActiveFloatingEntries.Add(Entry);
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

    static const FSlateColorBrush SolidWhiteBrush(FLinearColor::White);
    const FSlateBrush* DefaultBrush = &SolidWhiteBrush;
    APlayerController* PC = GetOwningPlayer();

    int32 ViewSizeX = FMath::RoundToInt(ViewportSize.X);
    int32 ViewSizeY = FMath::RoundToInt(ViewportSize.Y);
    if (PC)
    {
        PC->GetViewportSize(ViewSizeX, ViewSizeY);
    }
    const float InvScaleX = (ViewSizeX > 0) ? (ViewportSize.X / static_cast<float>(ViewSizeX)) : 1.0f;
    const float InvScaleY = (ViewSizeY > 0) ? (ViewportSize.Y / static_cast<float>(ViewSizeY)) : 1.0f;

    // 1. Animated Enemy Life Bars (Polished Mode Only — matches reference video)
    if (bPolishEnabled && PC)
    {
        for (const auto& Pair : EnemyLifeBars)
        {
            const TWeakObjectPtr<AGameFeelEnemyCharacter>& WeakEnemy = Pair.Key;
            const FEnemyLifeBarState& State = Pair.Value;
            if (!WeakEnemy.IsValid() || State.Alpha <= 0.01f)
            {
                continue;
            }

            const FVector WorldPos = State.LastWorldLocation + FVector(0.0f, 0.0f, 76.0f);
            FVector2D ScreenPixels = FVector2D::ZeroVector;
            if (!PC->ProjectWorldLocationToScreen(WorldPos, ScreenPixels, true))
            {
                continue;
            }

            FVector2D BarCenter(ScreenPixels.X * InvScaleX, ScreenPixels.Y * InvScaleY);
            if (State.ShakeTimer > 0.0f)
            {
                BarCenter.X += FMath::Sin(State.ShakeTimer * 110.0f) * 3.5f;
                BarCenter.Y += FMath::Cos(State.ShakeTimer * 85.0f) * 1.5f;
            }

            const float BarWidth = 46.0f * State.Scale;
            const float BarHeight = 5.5f * State.Scale;
            const FVector2D BarSize(BarWidth, BarHeight);
            const FVector2D BarTopLeft = BarCenter - (BarSize * 0.5f);

            // Outer border / dark backing
            const FVector2D BorderPad(1.5f, 1.5f);
            FSlateDrawElement::MakeBox(
                OutDrawElements,
                LayerId++,
                AllottedGeometry.ToPaintGeometry(BarSize + BorderPad * 2.0f, FSlateLayoutTransform(BarTopLeft - BorderPad)),
                DefaultBrush,
                ESlateDrawEffect::None,
                FLinearColor(0.02f, 0.02f, 0.03f, 0.94f * State.Alpha));

            // Empty dark track
            FSlateDrawElement::MakeBox(
                OutDrawElements,
                LayerId++,
                AllottedGeometry.ToPaintGeometry(BarSize, FSlateLayoutTransform(BarTopLeft)),
                DefaultBrush,
                ESlateDrawEffect::None,
                FLinearColor(0.07f, 0.06f, 0.08f, 0.95f * State.Alpha));

            // Delayed white damage catch-up bar
            const float TrailWidth = BarWidth * FMath::Clamp(State.TrailingPercent, 0.0f, 1.0f);
            if (TrailWidth > 0.5f)
            {
                FSlateDrawElement::MakeBox(
                    OutDrawElements,
                    LayerId++,
                    AllottedGeometry.ToPaintGeometry(FVector2D(TrailWidth, BarHeight), FSlateLayoutTransform(BarTopLeft)),
                    DefaultBrush,
                    ESlateDrawEffect::None,
                    FLinearColor(1.0f, 1.0f, 1.0f, 0.96f * State.Alpha));
            }

            // Current red health fill (base color)
            const float FillWidth = BarWidth * FMath::Clamp(State.DisplayedPercent, 0.0f, 1.0f);
            if (FillWidth > 0.5f)
            {
                FSlateDrawElement::MakeBox(
                    OutDrawElements,
                    LayerId++,
                    AllottedGeometry.ToPaintGeometry(FVector2D(FillWidth, BarHeight), FSlateLayoutTransform(BarTopLeft)),
                    DefaultBrush,
                    ESlateDrawEffect::None,
                    FLinearColor(0.92f, 0.14f, 0.12f, 1.0f * State.Alpha));
            }
        }
    }

    // 2. Floating Damage Numbers Only
    if (PC)
    {
        for (const FFloatingCombatEntry& Entry : ActiveFloatingEntries)
        {
            FVector2D ScreenPixels = FVector2D::ZeroVector;
            if (!PC->ProjectWorldLocationToScreen(Entry.WorldOrigin, ScreenPixels, true))
            {
                continue;
            }

            const FVector2D BasePos(ScreenPixels.X * InvScaleX, ScreenPixels.Y * InvScaleY);
            const FVector2D FinalPos = BasePos + Entry.ScreenOffset;

            const float NormTime = FMath::Clamp(Entry.Elapsed / Entry.Duration, 0.0f, 1.0f);
            const float Alpha = (NormTime < 0.72f)
                ? 1.0f
                : FMath::Clamp(1.0f - ((NormTime - 0.72f) / 0.28f), 0.0f, 1.0f);

            const int32 ScaledFontSize = FMath::Clamp(FMath::RoundToInt(Entry.BaseFontSize * Entry.Scale), 10, 56);
            const FSlateFontInfo DmgFont = FCoreStyle::GetDefaultFontStyle("Bold", ScaledFontSize);

            FLinearColor DmgColor = FLinearColor::White;
            DmgColor.A = Alpha;

            DrawOutlinedText(
                OutDrawElements,
                LayerId,
                AllottedGeometry,
                Entry.DamageText,
                DmgFont,
                FinalPos,
                DmgColor,
                Alpha,
                Entry.bPolished);
        }
    }

    // 3. Sleek Bottom Controls & Live Polish Status Bar
    {
        const FVector2D BarSize(510.0f, 30.0f);
        const FVector2D BarCenter(ViewportSize.X * 0.5f, ViewportSize.Y - 28.0f);
        const FVector2D BarTopLeft = BarCenter - (BarSize * 0.5f);

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId++,
            AllottedGeometry.ToPaintGeometry(BarSize, FSlateLayoutTransform(BarTopLeft)),
            DefaultBrush,
            ESlateDrawEffect::None,
            FLinearColor(0.02f, 0.02f, 0.04f, 0.62f));

        const FSlateFontInfo ControlsFont = FCoreStyle::GetDefaultFontStyle("Regular", 10);
        const FSlateFontInfo StatusFont = FCoreStyle::GetDefaultFontStyle("Bold", FMath::RoundToInt(11.0f * PolishBadgePulse));

        DrawOutlinedText(
            OutDrawElements,
            LayerId,
            AllottedGeometry,
            TEXT("[WASD] Move     [Mouse] Aim     [LMB] Attack"),
            ControlsFont,
            BarCenter - FVector2D(72.0f, 0.0f),
            FLinearColor(1.0f, 1.0f, 1.0f, 0.92f),
            0.6f,
            true);

        const FString StatusText = bPolishEnabled
            ? TEXT("[T] POLISH: ON")
            : TEXT("[T] POLISH: OFF");

        DrawOutlinedText(
            OutDrawElements,
            LayerId,
            AllottedGeometry,
            StatusText,
            StatusFont,
            BarCenter + FVector2D(168.0f, 0.0f),
            FLinearColor(1.0f, 1.0f, 1.0f, 0.98f),
            0.85f,
            true);
    }

    return LayerId;
}
