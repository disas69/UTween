#include "JuiceDummyActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "UTween.h"

AJuiceDummyActor::AJuiceDummyActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    UStaticMesh* CylinderMesh = CylinderFinder.Succeeded() ? CylinderFinder.Object : nullptr;
    UStaticMesh* SphereMesh   = SphereFinder.Succeeded() ? SphereFinder.Object : nullptr;
    UStaticMesh* CubeMesh     = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
    UMaterialInterface* BaseMat = MaterialFinder.Succeeded() ? MaterialFinder.Object : nullptr;

    BasePedestal = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BasePedestal"));
    BasePedestal->SetupAttachment(SceneRoot);
    BasePedestal->SetRelativeScale3D(FVector(1.4f, 1.4f, 0.2f));
    BasePedestal->SetMobility(EComponentMobility::Movable);
    if (CylinderMesh) BasePedestal->SetStaticMesh(CylinderMesh);
    if (BaseMat) BasePedestal->SetMaterial(0, BaseMat);

    WobblePivot = CreateDefaultSubobject<USceneComponent>(TEXT("WobblePivot"));
    WobblePivot->SetupAttachment(SceneRoot);
    WobblePivot->SetRelativeLocation(FVector(0.f, 0.f, 20.f));
    WobblePivot->SetMobility(EComponentMobility::Movable);

    StemPole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StemPole"));
    StemPole->SetupAttachment(WobblePivot);
    StemPole->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
    StemPole->SetRelativeScale3D(FVector(0.18f, 0.18f, 1.0f));
    StemPole->SetMobility(EComponentMobility::Movable);
    if (CylinderMesh) StemPole->SetStaticMesh(CylinderMesh);
    if (BaseMat) StemPole->SetMaterial(0, BaseMat);

    TorsoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TorsoMesh"));
    TorsoMesh->SetupAttachment(WobblePivot);
    TorsoMesh->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
    TorsoMesh->SetRelativeScale3D(FVector(0.85f, 0.85f, 1.05f));
    TorsoMesh->SetMobility(EComponentMobility::Movable);
    if (CubeMesh) TorsoMesh->SetStaticMesh(CubeMesh);
    if (BaseMat) TorsoMesh->SetMaterial(0, BaseMat);

    HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
    HeadMesh->SetupAttachment(WobblePivot);
    HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 205.f));
    HeadMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.55f));
    HeadMesh->SetMobility(EComponentMobility::Movable);
    if (SphereMesh) HeadMesh->SetStaticMesh(SphereMesh);
    if (BaseMat) HeadMesh->SetMaterial(0, BaseMat);

    TargetDisc = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetDisc"));
    TargetDisc->SetupAttachment(TorsoMesh);
    TargetDisc->SetRelativeLocation(FVector(48.f, 0.f, 0.f));
    TargetDisc->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
    TargetDisc->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.08f));
    TargetDisc->SetMobility(EComponentMobility::Movable);
    if (CylinderMesh) TargetDisc->SetStaticMesh(CylinderMesh);
    if (BaseMat) TargetDisc->SetMaterial(0, BaseMat);
}

void AJuiceDummyActor::BeginPlay()
{
    Super::BeginPlay();

    if (TorsoMesh)
    {
        TorsoDynMat = TorsoMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (TorsoDynMat)
        {
            TorsoDynMat->SetVectorParameterValue(FName("Color"), FLinearColor(0.08f, 0.12f, 0.22f, 1.0f));
        }
    }

    if (HeadMesh)
    {
        HeadDynMat = HeadMesh->CreateAndSetMaterialInstanceDynamic(0);
        if (HeadDynMat)
        {
            HeadDynMat->SetVectorParameterValue(FName("Color"), FLinearColor(0.6f, 0.65f, 0.7f, 1.0f));
        }
    }

    if (TargetDisc)
    {
        DiscDynMat = TargetDisc->CreateAndSetMaterialInstanceDynamic(0);
        if (DiscDynMat)
        {
            DiscDynMat->SetVectorParameterValue(FName("Color"), FLinearColor(1.0f, 0.45f, 0.05f, 1.0f));
        }
    }

    if (TorsoMesh)
    {
        UTween::ComponentRelativeLocation(TorsoMesh, FVector(0.f, 0.f, 134.f), 1.5f)
            .SetEase(ETweenEase::InOutSine)
            .SetLoops(-1, ETweenLoopMode::Yoyo);
    }
}

void AJuiceDummyActor::TakeHit(float Damage, FVector HitDirection, bool bIsCritical)
{
    const double Now = FPlatformTime::Seconds();
    if (Now - LastHitTime < 1.4)
    {
        CurrentCombo++;
    }
    else
    {
        CurrentCombo = 1;
    }
    LastHitTime = Now;

    const float ComboMultiplier = FMath::Clamp(1.0f + static_cast<float>(CurrentCombo - 1) * 0.12f, 1.0f, 2.2f);

    const FVector Squash = bIsCritical
        ? FVector(-0.35f, 0.35f, -0.45f) * ComboMultiplier
        : FVector(-0.2f, 0.2f, -0.25f) * ComboMultiplier;
    PunchScaleHandle.Kill();
    PunchScaleHandle = UTween::PunchScale(this, Squash, 0.38f, 12, 1.0f);

    const FVector Dir = HitDirection.IsNearlyZero() ? -GetActorForwardVector() : HitDirection.GetSafeNormal();
    const float RecoilDist = (bIsCritical ? 35.f : 18.f) * ComboMultiplier;
    PunchPositionHandle.Kill();
    PunchPositionHandle = UTween::PunchPosition(this, Dir * RecoilDist, 0.42f, 10, 0.8f);

    if (WobblePivot)
    {
        WobbleTweenHandle.Kill();
        const FRotator TiltRot(Dir.X * -18.f * ComboMultiplier, 0.f, Dir.Y * 18.f * ComboMultiplier);
        TWeakObjectPtr<AJuiceDummyActor> WeakThis(this);
        WobbleTweenHandle = UTween::ComponentRelativeRotation(WobblePivot, TiltRot, 0.12f)
            .SetEase(ETweenEase::OutQuad)
            .OnComplete([WeakThis]()
            {
                if (AJuiceDummyActor* Self = WeakThis.Get())
                {
                    if (Self->WobblePivot)
                    {
                        Self->WobbleTweenHandle = UTween::ComponentRelativeRotation(Self->WobblePivot, FRotator::ZeroRotator, 0.5f)
                            .SetEase(ETweenEase::OutElastic);
                    }
                }
            });
    }

    if (TorsoDynMat)
    {
        FlashTweenHandle.Kill();
        const FLinearColor FlashColor = bIsCritical
            ? FLinearColor(8.0f, 6.0f, 0.2f, 1.0f)
            : FLinearColor(6.0f, 0.2f, 0.2f, 1.0f);

        TWeakObjectPtr<AJuiceDummyActor> WeakThis(this);
        FlashTweenHandle = UTween::MaterialVector(TorsoDynMat, FName("Color"), FlashColor, 0.05f)
            .OnComplete([WeakThis]()
            {
                if (AJuiceDummyActor* Self = WeakThis.Get())
                {
                    if (Self->TorsoDynMat)
                    {
                        Self->FlashTweenHandle = UTween::MaterialVector(Self->TorsoDynMat, FName("Color"), FLinearColor(0.08f, 0.12f, 0.22f, 1.0f), 0.28f)
                            .SetEase(ETweenEase::OutQuad);
                    }
                }
            });
    }

    if (TargetDisc)
    {
        DiscScaleTweenHandle.Kill();
        TWeakObjectPtr<AJuiceDummyActor> WeakThis(this);
        DiscScaleTweenHandle = UTween::ComponentRelativeScale(TargetDisc, FVector(0.6f, 0.6f, 0.12f), 0.1f)
            .SetEase(ETweenEase::OutQuad)
            .OnComplete([WeakThis]()
            {
                if (AJuiceDummyActor* Self = WeakThis.Get())
                {
                    if (Self->TargetDisc)
                    {
                        Self->DiscScaleTweenHandle = UTween::ComponentRelativeScale(Self->TargetDisc, FVector(0.45f, 0.45f, 0.08f), 0.25f)
                            .SetEase(ETweenEase::OutBounce);
                    }
                }
            });
    }

    const FVector SpawnLoc = TargetDisc ? TargetDisc->GetComponentLocation() : (GetActorLocation() + FVector(0.f, 0.f, 130.f));
    OnDummyHit.Broadcast(Damage, SpawnLoc, bIsCritical, CurrentCombo);
}

void AJuiceDummyActor::ResetCombo()
{
    CurrentCombo = 0;
    LastHitTime = -10.0;
}
