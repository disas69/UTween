#include "UTweenDemoManager.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Styling/CoreStyle.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#include "UTween.h"
#include "UTweenMacros.h"

AUTweenDemoManager::AUTweenDemoManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> StudioCubeFinder(
        TEXT("/Game/TweenDemo/Meshes/SM_TweenDemo_Cube.SM_TweenDemo_Cube"));
    if (StudioCubeFinder.Succeeded())
    {
        ShapeMesh = StudioCubeFinder.Object;
    }
    else
    {
        static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
            TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeFinder.Succeeded())
        {
            ShapeMesh = CubeFinder.Object;
        }
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> StudioMatFinder(
        TEXT("/Game/TweenDemo/Materials/MI_TweenDemo_Cube.MI_TweenDemo_Cube"));
    if (StudioMatFinder.Succeeded())
    {
        ShapeMaterial = StudioMatFinder.Object;
    }
    else
    {
        static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
            TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
        if (MaterialFinder.Succeeded())
        {
            ShapeMaterial = MaterialFinder.Object;
        }
    }
}

void AUTweenDemoManager::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Collect the placed cube actors from the level (tagged DemoCube_0 .. DemoCube_9)
    SpawnedActors.Reset();
    SpawnedActors.SetNumZeroed(NumShapes);

    int32 FoundCount = 0;
    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* Candidate = *It;
        if (!Candidate)
        {
            continue;
        }

        for (int32 i = 0; i < NumShapes; ++i)
        {
            const FName ExpectedTag(*FString::Printf(TEXT("DemoCube_%d"), i));
            if (Candidate->ActorHasTag(ExpectedTag))
            {
                Candidate->SetMobility(EComponentMobility::Movable);
                SpawnedActors[i] = Candidate;
                ++FoundCount;
                break;
            }
        }
    }

    SpawnedLabels.Reset();
    SpawnedLabels.SetNumZeroed(NumShapes);

    const bool bUseBlueprintTweens = ActorHasTag(TEXT("UseBlueprintTweens"));

    for (int32 i = 0; i < NumShapes; ++i)
    {
        if (AStaticMeshActor* Shape = SpawnedActors[i])
        {
            SpawnedLabels[i] = SpawnLabelForShape(i, Shape);
            if (!bUseBlueprintTweens)
            {
                StartAnimationFor(i, Shape);
            }
        }
    }
}

void AUTweenDemoManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FRotator CamRot(-33.f, 0.f, 0.f);
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        if (PC->PlayerCameraManager)
        {
            CamRot = PC->PlayerCameraManager->GetCameraRotation();
        }
    }

    const FRotator LabelRot = (-CamRot.Vector()).Rotation();
    const int32 Count = FMath::Min(SpawnedActors.Num(), SpawnedLabels.Num());

    for (int32 i = 0; i < Count; ++i)
    {
        AStaticMeshActor* Shape = SpawnedActors[i];
        UWidgetComponent* Label = SpawnedLabels[i];
        if (!Shape || !Label)
        {
            continue;
        }

        const float CurrentScaleZ = FMath::Max(1.0f, Shape->GetActorScale3D().Z);
        const float VerticalOffset = 50.f * CurrentScaleZ + 44.f;
        Label->SetWorldLocation(Shape->GetActorLocation() + FVector(0.f, 0.f, VerticalOffset));
        Label->SetWorldRotation(LabelRot);
    }
}

FLinearColor AUTweenDemoManager::GetDemoColor(int32 Index) const
{
    static const FLinearColor Palette[10] = {
        FLinearColor(0.10f, 0.54f, 1.00f, 1.0f),
        FLinearColor(0.95f, 0.25f, 0.23f, 1.0f),
        FLinearColor(1.00f, 0.68f, 0.12f, 1.0f),
        FLinearColor(0.12f, 0.82f, 0.50f, 1.0f),
        FLinearColor(0.58f, 0.28f, 0.98f, 1.0f),
        FLinearColor(0.08f, 0.78f, 0.92f, 1.0f),
        FLinearColor(0.98f, 0.42f, 0.14f, 1.0f),
        FLinearColor(0.94f, 0.22f, 0.58f, 1.0f),
        FLinearColor(0.46f, 0.86f, 0.18f, 1.0f),
        FLinearColor(0.32f, 0.38f, 0.98f, 1.0f),
    };
    return Palette[Index % 10];
}

void AUTweenDemoManager::GetDemoText(int32 Index, FString& OutTitle, FString& OutSubtitle) const
{
    switch (Index % 10)
    {
    case 0:
        OutTitle    = TEXT("Bounce");
        OutSubtitle = TEXT("OutBounce + Yoyo");
        break;
    case 1:
        OutTitle    = TEXT("Spin");
        OutSubtitle = TEXT("360° InOutQuad");
        break;
    case 2:
        OutTitle    = TEXT("Scale Pulse");
        OutSubtitle = TEXT("InOutSine + Yoyo");
        break;
    case 3:
        OutTitle    = TEXT("Sequence");
        OutSubtitle = TEXT("4-Step Path");
        break;
    case 4:
        OutTitle    = TEXT("Elastic Pop");
        OutSubtitle = TEXT("OutElastic Scale");
        break;
    case 5:
        OutTitle    = TEXT("Barrel Roll");
        OutSubtitle = TEXT("360° InOutCubic");
        break;
    case 6:
        OutTitle    = TEXT("Value Tween");
        OutSubtitle = TEXT("Figure-8 Orbit");
        break;
    case 7:
        OutTitle    = TEXT("Parallel");
        OutSubtitle = TEXT("Jump + 720° Flip");
        break;
    case 8:
        OutTitle    = TEXT("Combo Tween");
        OutSubtitle = TEXT("Pos + Rot + Scale");
        break;
    case 9:
        OutTitle    = TEXT("Nested Tween");
        OutSubtitle = TEXT("Pop + Shake Loop");
        break;
    default:
        OutTitle    = TEXT("Tween");
        OutSubtitle = TEXT("Demo");
        break;
    }
}

AStaticMeshActor* AUTweenDemoManager::SpawnShape(int32 Index)
{
    return SpawnedActors.IsValidIndex(Index) ? SpawnedActors[Index] : nullptr;
}

UWidgetComponent* AUTweenDemoManager::SpawnLabelForShape(int32 Index, AStaticMeshActor* Shape)
{
    if (!Shape)
    {
        return nullptr;
    }

    const FName CompName(*FString::Printf(TEXT("DemoLabel_%02d"), Index));
    UWidgetComponent* LabelComp = NewObject<UWidgetComponent>(this, CompName);
    if (!LabelComp)
    {
        return nullptr;
    }

    const bool bHasPlayerScreen = (UGameplayStatics::GetPlayerController(this, 0) != nullptr);

    LabelComp->SetupAttachment(RootComponent);
    LabelComp->SetWidgetSpace(bHasPlayerScreen ? EWidgetSpace::Screen : EWidgetSpace::World);
    LabelComp->SetDrawAtDesiredSize(bHasPlayerScreen);
    LabelComp->SetDrawSize(FVector2D(520.f, 160.f));
    LabelComp->SetPivot(FVector2D(0.5f, 1.0f));
    LabelComp->SetBlendMode(EWidgetBlendMode::Transparent);
    LabelComp->SetBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
    LabelComp->SetTwoSided(true);
    LabelComp->SetTickWhenOffscreen(true);
    LabelComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LabelComp->SetCastShadow(false);
    LabelComp->RegisterComponent();

    LabelComp->SetWorldLocation(Shape->GetActorLocation() + FVector(0.f, 0.f, 94.f));
    LabelComp->SetWorldRotation(FRotator(33.f, 180.f, 0.f));
    LabelComp->SetWorldScale3D(FVector(0.45f, 0.45f, 0.45f));

    FString TitleStr;
    FString SubtitleStr;
    GetDemoText(Index, TitleStr, SubtitleStr);

    const int32 TitleSize = bHasPlayerScreen ? 15 : 32;
    const int32 SubSize   = bHasPlayerScreen ? 11 : 20;

    FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", TitleSize);
    TitleFont.OutlineSettings.OutlineSize = 2;
    TitleFont.OutlineSettings.OutlineColor = FLinearColor(0.02f, 0.02f, 0.04f, 0.96f);

    FSlateFontInfo SubtitleFont = FCoreStyle::GetDefaultFontStyle("Bold", SubSize);
    SubtitleFont.OutlineSettings.OutlineSize = 2;
    SubtitleFont.OutlineSettings.OutlineColor = FLinearColor(0.02f, 0.02f, 0.04f, 0.92f);

    TSharedRef<SWidget> SlateLabel =
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TitleStr))
            .Font(TitleFont)
            .ColorAndOpacity(FLinearColor::White)
            .ShadowOffset(FVector2D(0.f, 2.f))
            .ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f))
            .Justification(ETextJustify::Center)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0.f, 2.f, 0.f, 0.f)
        .HAlign(HAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromString(SubtitleStr))
            .Font(SubtitleFont)
            .ColorAndOpacity(FLinearColor(0.92f, 0.94f, 0.98f, 0.95f))
            .ShadowOffset(FVector2D(0.f, 1.5f))
            .ShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.75f))
            .Justification(ETextJustify::Center)
        ];

    LabelComp->SetSlateWidget(SlateLabel);
    LabelComp->RequestRenderUpdate();
    return LabelComp;
}

void AUTweenDemoManager::StartAnimationFor(int32 Index, AStaticMeshActor* Shape)
{
    if (!Shape)
    {
        return;
    }

    const FVector Base = Shape->GetActorLocation();

    switch (Index % 10)
    {
    case 0: Demo0_VerticalBounce(Shape, Base); break;
    case 1: Demo1_ContinuousSpin(Shape); break;
    case 2: Demo2_ScalePulse(Shape); break;
    case 3: Demo3_SlideSequence(Shape, Base); break;
    case 4: Demo4_ElasticPop(Shape); break;
    case 5: Demo5_BarrelRoll(Shape); break;
    case 6: Demo6_Orbit(Shape, Base); break;
    case 7: Demo7_JumpFlip(Shape, Base); break;
    case 8: Demo8_ParallelCombo(Shape, Base); break;
    case 9: Demo9_NestedShake(Shape, Base); break;
    default: break;
    }
}

void AUTweenDemoManager::Demo0_VerticalBounce(AStaticMeshActor* Shape, const FVector& Base)
{
    UTWEEN_POSITION(Shape, Base, Base + FVector(0.f, 0.f, 105.f), 1.2f)
        .SetEase(ETweenEase::OutBounce)
        .SetLoops(-1, ETweenLoopMode::Yoyo);
}

void AUTweenDemoManager::Demo1_ContinuousSpin(AStaticMeshActor* Shape)
{
    UTWEEN_ROTATION(Shape, FRotator::ZeroRotator, FRotator(0.f, 360.f, 0.f), 2.0f)
        .SetEase(ETweenEase::InOutQuad)
        .SetLoops(-1, ETweenLoopMode::Yoyo);
}

void AUTweenDemoManager::Demo2_ScalePulse(AStaticMeshActor* Shape)
{
    UTWEEN_SCALE(Shape, FVector(0.85f), FVector(1.3f), 0.8f)
        .SetEase(ETweenEase::InOutSine)
        .SetLoops(-1, ETweenLoopMode::Yoyo);
}

void AUTweenDemoManager::Demo3_SlideSequence(AStaticMeshActor* Shape, const FVector& Base)
{
    const FVector Right = Base + FVector(0.f,  55.f, 50.f);
    const FVector Up    = Base + FVector(0.f,   0.f, 95.f);
    const FVector Left  = Base + FVector(0.f, -55.f, 50.f);

    UTWEEN_SEQUENCE(Shape,
        UTWEEN_POSITION(Shape, Base,  Right, 0.5f).SetEase(ETweenEase::OutCubic),
        UTWEEN_POSITION(Shape, Right, Up,    0.5f).SetEase(ETweenEase::OutCubic),
        UTWEEN_POSITION(Shape, Up,    Left,  0.5f).SetEase(ETweenEase::OutCubic),
        UTWEEN_POSITION(Shape, Left,  Base,  0.5f).SetEase(ETweenEase::OutCubic))
        .SetLoops(-1, ETweenLoopMode::Restart)
        .Play();
}

void AUTweenDemoManager::Demo4_ElasticPop(AStaticMeshActor* Shape)
{
    UTWEEN_SCALE(Shape, FVector(0.f), FVector(1.f), 1.0f)
        .SetEase(ETweenEase::OutElastic)
        .SetLoops(-1, ETweenLoopMode::Yoyo);
}

void AUTweenDemoManager::Demo5_BarrelRoll(AStaticMeshActor* Shape)
{
    UTWEEN_ROTATION(Shape, FRotator::ZeroRotator, FRotator(0.f, 0.f, 360.f), 1.5f)
        .SetEase(ETweenEase::InOutCubic)
        .SetLoops(-1, ETweenLoopMode::Restart);
}

void AUTweenDemoManager::Demo6_Orbit(AStaticMeshActor* Shape, const FVector& Base)
{
    TWeakObjectPtr<AStaticMeshActor> WeakShape(Shape);
    UTWEEN(Shape, 0.f, 2.f * PI,
        ([WeakShape, Base](float Theta)
        {
            if (AStaticMeshActor* ValidShape = WeakShape.Get())
            {
                const FVector Offset(
                    30.f * FMath::Cos(Theta),
                    60.f * FMath::Sin(Theta),
                    42.f * FMath::Sin(2.f * Theta) + 45.f);
                ValidShape->SetActorLocation(Base + Offset);
            }
        }),
        3.f)
        .SetEase(ETweenEase::Linear)
        .SetLoops(-1, ETweenLoopMode::Restart);
}

void AUTweenDemoManager::Demo7_JumpFlip(AStaticMeshActor* Shape, const FVector& Base)
{
    const FVector Apex = Base + FVector(0.f, 0.f, 115.f);

    UTWEEN_PARALLEL(Shape,
        UTWEEN_POSITION(Shape, Base, Apex, 0.6f).SetEase(ETweenEase::OutQuad),
        UTWEEN_POSITION(Shape, Apex, Base, 0.6f).SetEase(ETweenEase::InQuad).SetDelay(0.6f),
        UTWEEN_ROTATION(Shape, FRotator::ZeroRotator, FRotator(360.f, 0.f, 360.f), 1.2f)
            .SetEase(ETweenEase::InOutCubic))
        .SetLoops(-1, ETweenLoopMode::Restart)
        .Play();
}

void AUTweenDemoManager::Demo8_ParallelCombo(AStaticMeshActor* Shape, const FVector& Base)
{
    const FVector High = Base + FVector(0.f, 0.f, 105.f);

    UTWEEN_PARALLEL(Shape,
        UTWEEN_POSITION(Shape, Base, High, 1.2f).SetEase(ETweenEase::InOutQuad),
        UTWEEN_ROTATION(Shape, FRotator::ZeroRotator, FRotator(0.f, 360.f, 180.f), 1.2f)
            .SetEase(ETweenEase::InOutQuad),
        UTWEEN_SCALE(Shape, FVector(0.8f), FVector(1.3f), 1.2f).SetEase(ETweenEase::InOutSine))
        .SetLoops(-1, ETweenLoopMode::Yoyo)
        .Play();
}

void AUTweenDemoManager::Demo9_NestedShake(AStaticMeshActor* Shape, const FVector& Base)
{
    FTweenSequenceHandle Shake = UTWEEN_SEQUENCE(Shape,
        UTWEEN_ROTATION(Shape, FRotator::ZeroRotator,    FRotator(0.f,  20.f, 0.f), 0.08f).SetEase(ETweenEase::InOutSine),
        UTWEEN_ROTATION(Shape, FRotator(0.f,  20.f, 0.f), FRotator(0.f, -20.f, 0.f), 0.16f).SetEase(ETweenEase::InOutSine),
        UTWEEN_ROTATION(Shape, FRotator(0.f, -20.f, 0.f), FRotator::ZeroRotator,     0.08f).SetEase(ETweenEase::InOutSine))
        .SetLoops(4, ETweenLoopMode::Restart);

    UTWEEN_SEQUENCE(Shape,
        UTWEEN_SCALE(Shape, FVector(0.f), FVector(1.f), 0.4f).SetEase(ETweenEase::OutElastic),
        UTWEEN_WAIT(0.2f),
        UTWEEN_DO([Shake]() mutable { Shake.Play(); }),
        UTWEEN_WAIT(1.6f),
        UTWEEN_SCALE(Shape, FVector(1.f), FVector(0.f), 0.3f).SetEase(ETweenEase::InBack),
        UTWEEN_WAIT(0.3f))
        .SetLoops(-1, ETweenLoopMode::Restart)
        .Play();

    (void)Base;
}
