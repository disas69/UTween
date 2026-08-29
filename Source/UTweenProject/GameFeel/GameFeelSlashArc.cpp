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
    ArcMesh->SetRelativeLocation(FVector(40.f, 0.f, 5.f));
    ArcMesh->SetRelativeScale3D(FVector(0.01f, 0.01f, 0.01f));
    ArcMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ArcMesh->SetVisibility(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderFinder.Succeeded())
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

void AGameFeelSlashArc::TriggerSlash(float Duration)
{
    if (!ArcMesh)
    {
        return;
    }

    ArcMesh->SetVisibility(true);
    ArcMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.035f));

    // Scale outward quickly using UTween
    UTween::ComponentRelativeScale(ArcMesh, MaxSlashScale, Duration)
        .SetEase(ETweenEase::OutQuad);

    // Emissive white flash & rapid fade to black
    if (DynamicSlashMaterial)
    {
        DynamicSlashMaterial->SetVectorParameterValue(FName("EmissiveColor"), SlashEmissiveColor);
        UTween::MaterialVector(DynamicSlashMaterial, FName("EmissiveColor"), FLinearColor::Black, Duration)
            .SetEase(ETweenEase::InQuad);
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