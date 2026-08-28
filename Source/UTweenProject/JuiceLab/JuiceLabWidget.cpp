#include "JuiceLabWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Fonts/SlateFontInfo.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/SlateColor.h"
#include "UTween.h"

#include "JuiceChestActor.h"
#include "JuiceDummyActor.h"

UJuiceLabWidget::UJuiceLabWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UJuiceLabWidget::RebuildWidget()
{
    if (!WidgetTree)
    {
        WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
    }

    if (!WidgetTree->RootWidget)
    {
        RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
        WidgetTree->RootWidget = RootCanvas;
    }
    else if (!RootCanvas)
    {
        RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
    }

    // Try finding existing Blueprint widgets
    if (!PrimaryHealthBar)
    {
        PrimaryHealthBar = WidgetTree->FindWidget<UProgressBar>(TEXT("Bar_PrimaryHealth"));
    }
    if (!GhostHealthBar)
    {
        GhostHealthBar = WidgetTree->FindWidget<UProgressBar>(TEXT("Bar_GhostHealth"));
    }
    if (!HealthText)
    {
        HealthText = WidgetTree->FindWidget<UTextBlock>(TEXT("Text_Health"));
    }
    if (!ComboText)
    {
        ComboText = WidgetTree->FindWidget<UTextBlock>(TEXT("Text_Combo"));
    }
    if (!TimeDilationLabel)
    {
        TimeDilationLabel = WidgetTree->FindWidget<UTextBlock>(TEXT("Text_TimeTitle"));
    }
    if (!DamageNumberCanvas)
    {
        DamageNumberCanvas = WidgetTree->FindWidget<UCanvasPanel>(TEXT("DamageNumberCanvas"));
    }

    // If still no health bar and canvas exists with 0 children, generate fallback
    if (!PrimaryHealthBar && RootCanvas && RootCanvas->GetChildrenCount() == 0)
    {
        BuildProceduralUI();
    }
    else if (!DamageNumberCanvas && RootCanvas)
    {
        DamageNumberCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DamageNumberCanvas"));
        if (UCanvasPanelSlot* DamageSlot = RootCanvas->AddChildToCanvas(DamageNumberCanvas))
        {
            DamageSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
            DamageSlot->SetOffsets(FMargin(0.f));
        }
    }

    return Super::RebuildWidget();
}

void UJuiceLabWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UJuiceLabWidget::BuildProceduralUI()
{
    if (!RootCanvas)
    {
        RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
        WidgetTree->RootWidget = RootCanvas;
    }

    DamageNumberCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DamageNumberCanvas"));
    if (UCanvasPanelSlot* DamageSlot = RootCanvas->AddChildToCanvas(DamageNumberCanvas))
    {
        DamageSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
        DamageSlot->SetOffsets(FMargin(0.f));
    }

    // Top-Left: Health bar and ghost bar
    UVerticalBox* HealthBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HealthBox"));
    HealthContainer = HealthBox;

    UTextBlock* DummyTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DummyTitle"));
    DummyTitle->SetText(FText::FromString(TEXT("TRAINING TARGET DUMMY")));
    DummyTitle->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.0f)));
    FSlateFontInfo TitleFont = DummyTitle->GetFont();
    TitleFont.Size = 13;
    DummyTitle->SetFont(TitleFont);
    HealthBox->AddChildToVerticalBox(DummyTitle);

    UOverlay* BarOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("BarOverlay"));

    GhostHealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("GhostHealthBar"));
    GhostHealthBar->SetPercent(1.0f);
    GhostHealthBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.15f, 0.8f));
    if (UOverlaySlot* GhostSlot = BarOverlay->AddChildToOverlay(GhostHealthBar))
    {
        GhostSlot->SetHorizontalAlignment(HAlign_Fill);
        GhostSlot->SetVerticalAlignment(VAlign_Fill);
    }

    PrimaryHealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("PrimaryHealthBar"));
    PrimaryHealthBar->SetPercent(1.0f);
    PrimaryHealthBar->SetFillColorAndOpacity(FLinearColor(0.15f, 0.85f, 0.35f, 1.0f));
    if (UOverlaySlot* PrimSlot = BarOverlay->AddChildToOverlay(PrimaryHealthBar))
    {
        PrimSlot->SetHorizontalAlignment(HAlign_Fill);
        PrimSlot->SetVerticalAlignment(VAlign_Fill);
    }

    HealthText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HealthText"));
    HealthText->SetText(FText::FromString(TEXT("1000 / 1000 HP")));
    HealthText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    FSlateFontInfo HpFont = HealthText->GetFont();
    HpFont.Size = 12;
    HealthText->SetFont(HpFont);
    if (UOverlaySlot* TextSlot = BarOverlay->AddChildToOverlay(HealthText))
    {
        TextSlot->SetHorizontalAlignment(HAlign_Center);
        TextSlot->SetVerticalAlignment(VAlign_Center);
    }

    if (UVerticalBoxSlot* BarSlot = HealthBox->AddChildToVerticalBox(BarOverlay))
    {
        BarSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 0.f));
    }

    if (UCanvasPanelSlot* HealthCanvasSlot = RootCanvas->AddChildToCanvas(HealthBox))
    {
        HealthCanvasSlot->SetPosition(FVector2D(40.f, 35.f));
        HealthCanvasSlot->SetSize(FVector2D(340.f, 50.f));
    }

    // Top-Center: Combo text
    ComboText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ComboText"));
    ComboText->SetText(FText::FromString(TEXT("")));
    ComboText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.05f, 1.0f)));
    FSlateFontInfo ComboFont = ComboText->GetFont();
    ComboFont.Size = 36;
    ComboText->SetFont(ComboFont);
    ComboText->SetRenderOpacity(0.0f);

    if (UCanvasPanelSlot* ComboSlot = RootCanvas->AddChildToCanvas(ComboText))
    {
        ComboSlot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
        ComboSlot->SetAlignment(FVector2D(0.5f, 0.f));
        ComboSlot->SetPosition(FVector2D(0.f, 40.f));
        ComboSlot->SetAutoSize(true);
    }

    // Top-Right: Time Dilation & RealTime test panel
    UVerticalBox* TimeBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TimeBox"));

    TimeDilationLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TimeDilationLabel"));
    TimeDilationLabel->SetText(FText::FromString(TEXT("Time Dilation: 1.0x (Normal)")));
    TimeDilationLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f)));
    FSlateFontInfo TimeFont = TimeDilationLabel->GetFont();
    TimeFont.Size = 13;
    TimeDilationLabel->SetFont(TimeFont);
    TimeBox->AddChildToVerticalBox(TimeDilationLabel);

    UTextBlock* SubLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SubLabel"));
    SubLabel->SetText(FText::FromString(TEXT("UTween RealTime keeps UI animating smoothly!")));
    SubLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.65f, 0.65f, 1.0f)));
    FSlateFontInfo SubFont = SubLabel->GetFont();
    SubFont.Size = 11;
    SubLabel->SetFont(SubFont);
    TimeBox->AddChildToVerticalBox(SubLabel);

    UHorizontalBox* SpeedBtns = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SpeedBtns"));

    auto MakeSpeedBtn = [this, SpeedBtns](const FString& Text, TObjectPtr<UButton>& OutBtn, FLinearColor Color)
    {
        OutBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
        OutBtn->SetBackgroundColor(Color);
        UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        Label->SetText(FText::FromString(Text));
        FSlateFontInfo F = Label->GetFont();
        F.Size = 12;
        Label->SetFont(F);
        OutBtn->AddChild(Label);
        if (UHorizontalBoxSlot* S = SpeedBtns->AddChildToHorizontalBox(OutBtn))
        {
            S->SetPadding(FMargin(0.f, 5.f, 8.f, 0.f));
        }
    };

    MakeSpeedBtn(TEXT("1.0x"), BtnSpeed1, FLinearColor(0.2f, 0.5f, 0.2f, 1.0f));
    MakeSpeedBtn(TEXT("0.1x (Slomo)"), BtnSpeed01, FLinearColor(0.5f, 0.35f, 0.1f, 1.0f));
    MakeSpeedBtn(TEXT("0.0x (Pause)"), BtnSpeed0, FLinearColor(0.55f, 0.15f, 0.15f, 1.0f));

    TimeBox->AddChildToVerticalBox(SpeedBtns);

    if (UCanvasPanelSlot* TimeSlot = RootCanvas->AddChildToCanvas(TimeBox))
    {
        TimeSlot->SetAnchors(FAnchors(1.f, 0.f, 1.f, 0.f));
        TimeSlot->SetAlignment(FVector2D(1.f, 0.f));
        TimeSlot->SetPosition(FVector2D(-40.f, 35.f));
        TimeSlot->SetAutoSize(true);
    }

    // Bottom-Center: Action Bar
    UHorizontalBox* ActionBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ActionBox"));

    auto MakeActionBtn = [this, ActionBox](const FString& Text, TObjectPtr<UButton>& OutBtn, FLinearColor Color)
    {
        OutBtn = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
        OutBtn->SetBackgroundColor(Color);
        UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        Label->SetText(FText::FromString(Text));
        FSlateFontInfo F = Label->GetFont();
        F.Size = 14;
        Label->SetFont(F);
        OutBtn->AddChild(Label);
        if (UHorizontalBoxSlot* S = ActionBox->AddChildToHorizontalBox(OutBtn))
        {
            S->SetPadding(FMargin(6.f, 0.f, 6.f, 0.f));
        }
    };

    MakeActionBtn(TEXT("⚔️ Light Hit (-50)"), BtnLight, FLinearColor(0.18f, 0.35f, 0.6f, 1.0f));
    MakeActionBtn(TEXT("💥 Heavy Hit (-150)"), BtnHeavy, FLinearColor(0.7f, 0.35f, 0.1f, 1.0f));
    MakeActionBtn(TEXT("⚡ Critical Smash (-450)"), BtnCrit, FLinearColor(0.8f, 0.15f, 0.15f, 1.0f));
    MakeActionBtn(TEXT("🎁 Open / Close Chest"), BtnChest, FLinearColor(0.45f, 0.2f, 0.6f, 1.0f));
    MakeActionBtn(TEXT("🔄 Reset Target"), BtnReset, FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));

    if (UCanvasPanelSlot* ActionSlot = RootCanvas->AddChildToCanvas(ActionBox))
    {
        ActionSlot->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
        ActionSlot->SetAlignment(FVector2D(0.5f, 1.f));
        ActionSlot->SetPosition(FVector2D(0.f, -40.f));
        ActionSlot->SetAutoSize(true);
    }

    if (BtnLight)   BtnLight->OnClicked.AddDynamic(this, &UJuiceLabWidget::HandleLightAttack);
    if (BtnHeavy)   BtnHeavy->OnClicked.AddDynamic(this, &UJuiceLabWidget::HandleHeavyAttack);
    if (BtnCrit)    BtnCrit->OnClicked.AddDynamic(this, &UJuiceLabWidget::HandleCritAttack);
    if (BtnChest)   BtnChest->OnClicked.AddDynamic(this, &UJuiceLabWidget::HandleToggleChest);
    if (BtnReset)   BtnReset->OnClicked.AddDynamic(this, &UJuiceLabWidget::HandleReset);
    if (BtnSpeed1)  BtnSpeed1->OnClicked.AddDynamic(this, &UJuiceLabWidget::HandleNormalSpeed);
    if (BtnSpeed01) BtnSpeed01->OnClicked.AddDynamic(this, &UJuiceLabWidget::HandleSlomoSpeed);
    if (BtnSpeed0)  BtnSpeed0->OnClicked.AddDynamic(this, &UJuiceLabWidget::HandlePauseSpeed);
}

void UJuiceLabWidget::SetTargetActors(AJuiceDummyActor* InDummy, AJuiceChestActor* InChest)
{
    DummyActor = InDummy;
    ChestActor = InChest;

    if (DummyActor)
    {
        DummyActor->OnDummyHit.AddDynamic(this, &UJuiceLabWidget::SpawnDamageNumber);
    }
}

void UJuiceLabWidget::ApplyDamage(float Damage, bool bIsCritical)
{
    CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
    const float NewPercent = CurrentHealth / MaxHealth;

    if (HealthText)
    {
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f HP"), CurrentHealth, MaxHealth)));
    }

    if (HealthContainer)
    {
        UTween::PunchScale(HealthContainer, FVector2D(0.06f, 0.12f), 0.22f, 10)
            .SetTimeMode(ETweenTimeMode::RealTime);
    }

    const float OldPrim = CurrentHealthPercent;
    UTween::Value(this, OldPrim, NewPercent, 0.18f, [this](float Val)
    {
        CurrentHealthPercent = Val;
        if (PrimaryHealthBar)
        {
            PrimaryHealthBar->SetPercent(Val);
        }
    })
    .SetEase(ETweenEase::OutQuad)
    .SetTimeMode(ETweenTimeMode::RealTime);

    const float OldGhost = GhostHealthPercent;
    UTween::Value(this, OldGhost, NewPercent, 0.45f, [this](float Val)
    {
        GhostHealthPercent = Val;
        if (GhostHealthBar)
        {
            GhostHealthBar->SetPercent(Val);
        }
    })
    .SetDelay(0.32f)
    .SetEase(ETweenEase::OutCubic)
    .SetTimeMode(ETweenTimeMode::RealTime);

    if (DummyActor)
    {
        const FVector HitDir = FVector(-1.f, FMath::RandRange(-0.3f, 0.3f), 0.f);
        DummyActor->TakeHit(Damage, HitDir, bIsCritical);
        UpdateComboText(DummyActor->CurrentCombo);
    }
}

void UJuiceLabWidget::ResetHealth()
{
    CurrentHealth = MaxHealth;
    CurrentHealthPercent = 1.0f;
    GhostHealthPercent = 1.0f;

    if (PrimaryHealthBar) PrimaryHealthBar->SetPercent(1.0f);
    if (GhostHealthBar)   GhostHealthBar->SetPercent(1.0f);
    if (HealthText)       HealthText->SetText(FText::FromString(TEXT("1000 / 1000 HP")));

    if (DummyActor)
    {
        DummyActor->ResetCombo();
        UpdateComboText(0);
    }
}

void UJuiceLabWidget::SpawnDamageNumber(float Damage, const FVector& WorldLocation, bool bIsCritical, int32 ComboCount)
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC || !DamageNumberCanvas || !WidgetTree)
    {
        return;
    }

    FVector2D ScreenPos;
    if (!UGameplayStatics::ProjectWorldToScreen(PC, WorldLocation, ScreenPos, true))
    {
        return;
    }

    ScreenPos.X += FMath::RandRange(-25.f, 25.f);
    ScreenPos.Y += FMath::RandRange(-15.f, 15.f);

    UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    if (!Text)
    {
        return;
    }

    const FString Prefix = bIsCritical ? TEXT("CRIT! -") : TEXT("-");
    Text->SetText(FText::FromString(FString::Printf(TEXT("%s%.0f"), *Prefix, Damage)));

    const FLinearColor TextColor = bIsCritical
        ? FLinearColor(1.0f, 0.85f, 0.1f, 1.0f)
        : FLinearColor(1.0f, 0.35f, 0.35f, 1.0f);
    Text->SetColorAndOpacity(FSlateColor(TextColor));

    FSlateFontInfo Font = Text->GetFont();
    Font.Size = bIsCritical ? 34 : 22;
    Text->SetFont(Font);

    UCanvasPanelSlot* DmgSlot = DamageNumberCanvas->AddChildToCanvas(Text);
    if (!DmgSlot)
    {
        return;
    }

    DmgSlot->SetPosition(ScreenPos);
    DmgSlot->SetAutoSize(true);

    Text->SetRenderScale(FVector2D(0.2f, 0.2f));
    UTween::WidgetScale(Text, bIsCritical ? FVector2D(1.4f, 1.4f) : FVector2D(1.0f, 1.0f), 0.28f)
        .SetEase(ETweenEase::OutBack)
        .SetTimeMode(ETweenTimeMode::RealTime);

    const FVector2D DriftTarget = ScreenPos - FVector2D(0.f, bIsCritical ? 110.f : 75.f);
    UTween::CanvasPosition(DmgSlot, DriftTarget, 0.85f)
        .SetEase(ETweenEase::OutCubic)
        .SetTimeMode(ETweenTimeMode::RealTime);

    UTween::WidgetOpacity(Text, 0.f, 0.3f)
        .SetDelay(0.5f)
        .SetEase(ETweenEase::InQuad)
        .SetTimeMode(ETweenTimeMode::RealTime)
        .OnComplete([WeakText = TWeakObjectPtr<UTextBlock>(Text)]()
        {
            if (WeakText.IsValid())
            {
                WeakText->RemoveFromParent();
            }
        });
}

void UJuiceLabWidget::UpdateComboText(int32 ComboCount)
{
    if (!ComboText)
    {
        return;
    }

    if (ComboCount <= 1)
    {
        ComboText->SetRenderOpacity(0.f);
        return;
    }

    ComboText->SetText(FText::FromString(FString::Printf(TEXT("%d COMBO!"), ComboCount)));
    ComboText->SetRenderOpacity(1.f);

    UTween::PunchScale(ComboText, FVector2D(0.35f, 0.35f), 0.3f, 10)
        .SetTimeMode(ETweenTimeMode::RealTime);
}

void UJuiceLabWidget::SetTimeDilation(float NewTimeDilation)
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), NewTimeDilation);

    if (TimeDilationLabel)
    {
        FString ModeStr = TEXT("1.0x (Normal)");
        if (FMath::IsNearlyZero(NewTimeDilation))
        {
            ModeStr = TEXT("0.0x (Paused - RealTime UI still active!)");
        }
        else if (NewTimeDilation < 0.5f)
        {
            ModeStr = TEXT("0.1x (Matrix Slomo)");
        }
        TimeDilationLabel->SetText(FText::FromString(FString::Printf(TEXT("Time Dilation: %s"), *ModeStr)));
    }
}

void UJuiceLabWidget::HandleLightAttack()
{
    if (BtnLight)
    {
        UTween::PunchScale(BtnLight, FVector2D(-0.15f, 0.15f), 0.2f, 8).SetTimeMode(ETweenTimeMode::RealTime);
    }
    ApplyDamage(50.f, false);
}

void UJuiceLabWidget::HandleHeavyAttack()
{
    if (BtnHeavy)
    {
        UTween::PunchScale(BtnHeavy, FVector2D(-0.18f, 0.18f), 0.22f, 10).SetTimeMode(ETweenTimeMode::RealTime);
    }
    ApplyDamage(150.f, false);
}

void UJuiceLabWidget::HandleCritAttack()
{
    if (BtnCrit)
    {
        UTween::PunchScale(BtnCrit, FVector2D(-0.25f, 0.25f), 0.25f, 12).SetTimeMode(ETweenTimeMode::RealTime);
    }
    ApplyDamage(450.f, true);
}

void UJuiceLabWidget::HandleToggleChest()
{
    if (BtnChest)
    {
        UTween::PunchScale(BtnChest, FVector2D(-0.15f, 0.15f), 0.2f, 8).SetTimeMode(ETweenTimeMode::RealTime);
    }
    if (ChestActor)
    {
        ChestActor->ToggleChest();
    }
}

void UJuiceLabWidget::HandleReset()
{
    if (BtnReset)
    {
        UTween::PunchScale(BtnReset, FVector2D(-0.12f, 0.12f), 0.18f, 8).SetTimeMode(ETweenTimeMode::RealTime);
    }
    ResetHealth();
}

void UJuiceLabWidget::HandleNormalSpeed()
{
    if (BtnSpeed1)
    {
        UTween::PunchScale(BtnSpeed1, FVector2D(-0.12f, 0.12f), 0.18f, 8).SetTimeMode(ETweenTimeMode::RealTime);
    }
    SetTimeDilation(1.0f);
}

void UJuiceLabWidget::HandleSlomoSpeed()
{
    if (BtnSpeed01)
    {
        UTween::PunchScale(BtnSpeed01, FVector2D(-0.12f, 0.12f), 0.18f, 8).SetTimeMode(ETweenTimeMode::RealTime);
    }
    SetTimeDilation(0.1f);
}

void UJuiceLabWidget::HandlePauseSpeed()
{
    if (BtnSpeed0)
    {
        UTween::PunchScale(BtnSpeed0, FVector2D(-0.12f, 0.12f), 0.18f, 8).SetTimeMode(ETweenTimeMode::RealTime);
    }
    SetTimeDilation(0.0f);
}
