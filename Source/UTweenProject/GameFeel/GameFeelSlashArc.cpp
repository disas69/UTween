#include "GameFeelSlashArc.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

#include "UTween.h"

AGameFeelSlashArc::AGameFeelSlashArc()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    SetRootComponent(RootComp);

    ArcMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArcMesh"));
    ArcMesh->SetupAttachment(RootComp);
    ArcMesh->SetRelativeLocation(FVector(8.f, 0.f, 8.f));
    ArcMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.01f));
    ArcMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ArcMesh->SetCastShadow(false);
    ArcMesh->SetVisibility(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SlashMeshFinder(TEXT("/Game/GameFeel/Meshes/SM_GameFeel_DoubleSlashArc.SM_GameFeel_DoubleSlashArc"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (SlashMeshFinder.Succeeded())
    {
        ArcMesh->SetStaticMesh(SlashMeshFinder.Object);
    }
    else if (CylinderFinder.Succeeded())
    {
        ArcMesh->SetStaticMesh(CylinderFinder.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SlashMatFinder(TEXT("/Game/GameFeel/Materials/MI_GameFeel_SlashArc.MI_GameFeel_SlashArc"));
    if (SlashMatFinder.Succeeded())
    {
        ArcMesh->SetMaterial(0, SlashMatFinder.Object);
    }
}

void AGameFeelSlashArc::BeginPlay()
{
    Super::BeginPlay();

    if (ArcMesh)
    {
        DynamicSlashMaterial = ArcMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicSlashMaterial)
        {
            DynamicSlashMaterial->SetVectorParameterValue(FName("BaseColor"), FLinearColor::White);
            DynamicSlashMaterial->SetVectorParameterValue(FName("EmissiveColor"), FLinearColor::Black);
        }
    }
}

void AGameFeelSlashArc::TriggerSlash(float Duration, FLinearColor CustomSlashColor)
{
    if (!ArcMesh)
    {
        return;
    }

    if (!DynamicSlashMaterial)
    {
        DynamicSlashMaterial = ArcMesh->CreateAndSetMaterialInstanceDynamic(0);
    }

    ArcMesh->SetHiddenInGame(false);
    ArcMesh->SetVisibility(true, true);
    ArcMesh->SetRelativeLocation(FVector(4.f, -8.f, 14.f));
    ArcMesh->SetRelativeScale3D(MaxSlashScale);
    ArcMesh->SetRelativeRotation(FRotator(0.f, -22.f, 0.f));

    // Scale and sweep outward quickly using UTween
    UTween::ComponentRelativeScale(ArcMesh, MaxSlashScale * 1.04f, Duration * 0.55f)
        .SetEase(ETweenEase::OutQuad);
    UTween::ComponentRelativeRotation(ArcMesh, FRotator(0.f, -14.f, 0.f), Duration)
        .SetEase(ETweenEase::OutQuad);

    const FLinearColor ActiveColor = (CustomSlashColor.R >= 0.f) ? CustomSlashColor : SlashEmissiveColor;

    // Emissive colored flash & rapid fade to black
    if (DynamicSlashMaterial)
    {
        DynamicSlashMaterial->SetVectorParameterValue(FName("EmissiveColor"), ActiveColor);
        UTween::Value(this, 0.f, 1.f, Duration * 0.55f, [](float) {})
            .OnComplete([this, Duration]() {
                if (DynamicSlashMaterial)
                {
                    UTween::MaterialVector(DynamicSlashMaterial, FName("EmissiveColor"), FLinearColor::Black, Duration * 0.45f)
                        .SetEase(ETweenEase::InQuad);
                }
            });
    }

    // Shrink out and hide when finished
    UTween::Value(this, 0.f, 1.f, Duration, [](float) {})
        .OnComplete([this]() {
            if (ArcMesh)
            {
                ArcMesh->SetVisibility(false);
            }
        });

    OnSlashTriggered(Duration);
}