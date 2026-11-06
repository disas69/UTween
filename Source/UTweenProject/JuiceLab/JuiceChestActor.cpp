#include "JuiceChestActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "UTween.h"

AJuiceChestActor::AJuiceChestActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    UStaticMesh* CubeMesh     = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
    UStaticMesh* SphereMesh   = SphereFinder.Succeeded() ? SphereFinder.Object : nullptr;
    UStaticMesh* CylinderMesh = CylinderFinder.Succeeded() ? CylinderFinder.Object : nullptr;
    UMaterialInterface* BaseMat = MaterialFinder.Succeeded() ? MaterialFinder.Object : nullptr;

    ChestBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestBase"));
    ChestBase->SetupAttachment(SceneRoot);
    ChestBase->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
    ChestBase->SetRelativeScale3D(FVector(1.0f, 1.4f, 0.6f));
    ChestBase->SetMobility(EComponentMobility::Movable);
    if (CubeMesh) ChestBase->SetStaticMesh(CubeMesh);
    if (BaseMat) ChestBase->SetMaterial(0, BaseMat);

    LidHinge = CreateDefaultSubobject<USceneComponent>(TEXT("LidHinge"));
    LidHinge->SetupAttachment(SceneRoot);
    LidHinge->SetRelativeLocation(FVector(-50.f, 0.f, 60.f));
    LidHinge->SetMobility(EComponentMobility::Movable);

    ChestLid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestLid"));
    ChestLid->SetupAttachment(LidHinge);
    ChestLid->SetRelativeLocation(FVector(50.f, 0.f, 15.f));
    ChestLid->SetRelativeScale3D(FVector(1.04f, 1.44f, 0.3f));
    ChestLid->SetMobility(EComponentMobility::Movable);
    if (CubeMesh) ChestLid->SetStaticMesh(CubeMesh);
    if (BaseMat) ChestLid->SetMaterial(0, BaseMat);

    LockGem = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockGem"));
    LockGem->SetupAttachment(ChestLid);
    LockGem->SetRelativeLocation(FVector(53.f, 0.f, -5.f));
    LockGem->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.2f));
    LockGem->SetMobility(EComponentMobility::Movable);
    if (SphereMesh) LockGem->SetStaticMesh(SphereMesh);
    if (BaseMat) LockGem->SetMaterial(0, BaseMat);

    LootContainer = CreateDefaultSubobject<USceneComponent>(TEXT("LootContainer"));
    LootContainer->SetupAttachment(SceneRoot);
    LootContainer->SetRelativeLocation(FVector(0.f, 0.f, 40.f));
    LootContainer->SetMobility(EComponentMobility::Movable);

    const int32 NumLoot = 4;
    LootItems.Reserve(NumLoot);
    for (int32 i = 0; i < NumLoot; ++i)
    {
        const FName ItemName = *FString::Printf(TEXT("LootItem_%d"), i);
        UStaticMeshComponent* Item = CreateDefaultSubobject<UStaticMeshComponent>(ItemName);
        Item->SetupAttachment(LootContainer);
        Item->SetRelativeLocation(FVector::ZeroVector);
        Item->SetRelativeScale3D(FVector::ZeroVector);
        Item->SetMobility(EComponentMobility::Movable);
        if (i % 2 == 0)
        {
            if (SphereMesh) Item->SetStaticMesh(SphereMesh);
        }
        else
        {
            if (CylinderMesh) Item->SetStaticMesh(CylinderMesh);
        }
        if (BaseMat) Item->SetMaterial(0, BaseMat);
        LootItems.Add(Item);
    }
}

void AJuiceChestActor::BeginPlay()
{
    Super::BeginPlay();

    if (ChestBase)
    {
        if (UMaterialInstanceDynamic* BaseDyn = ChestBase->CreateAndSetMaterialInstanceDynamic(0))
        {
            BaseDyn->SetVectorParameterValue(FName("Color"), FLinearColor(0.25f, 0.15f, 0.08f, 1.0f));
        }
    }

    if (ChestLid)
    {
        if (UMaterialInstanceDynamic* LidDyn = ChestLid->CreateAndSetMaterialInstanceDynamic(0))
        {
            LidDyn->SetVectorParameterValue(FName("Color"), FLinearColor(0.35f, 0.22f, 0.12f, 1.0f));
        }
    }

    if (LockGem)
    {
        LockDynMat = LockGem->CreateAndSetMaterialInstanceDynamic(0);
        if (LockDynMat)
        {
            LockDynMat->SetVectorParameterValue(FName("Color"), FLinearColor(1.0f, 0.8f, 0.1f, 1.0f));
        }
    }

    const FLinearColor GemColors[] = {
        FLinearColor(1.0f, 0.2f, 0.3f, 1.0f),
        FLinearColor(0.2f, 0.8f, 1.0f, 1.0f),
        FLinearColor(1.0f, 0.85f, 0.1f, 1.0f),
        FLinearColor(0.3f, 1.0f, 0.4f, 1.0f)
    };

    LootDynMats.Reset();
    for (int32 i = 0; i < LootItems.Num(); ++i)
    {
        if (LootItems[i])
        {
            UMaterialInstanceDynamic* DynMat = LootItems[i]->CreateAndSetMaterialInstanceDynamic(0);
            if (DynMat)
            {
                DynMat->SetVectorParameterValue(FName("Color"), GemColors[i % 4]);
            }
            LootDynMats.Add(DynMat);
        }
    }
}

void AJuiceChestActor::ToggleChest()
{
    if (bIsOpen)
    {
        CloseChest();
    }
    else
    {
        OpenChest();
    }
}

void AJuiceChestActor::OpenChest()
{
    if (bIsAnimating || bIsOpen)
    {
        return;
    }

    bIsAnimating = true;
    ChestSequenceHandle.Kill();

    FTweenSequenceHandle Seq = UTween::Sequence().SetBoundTarget(this);
    Seq.Append(UTween::ShakePosition(this, 5.0f, 0.3f, 16));

    if (LockDynMat)
    {
        Seq.Join(UTween::MaterialVector(LockDynMat, FName("Color"), FLinearColor(4.0f, 3.0f, 0.5f, 1.0f), 0.3f));
    }

    TWeakObjectPtr<AJuiceChestActor> WeakThis(this);
    Seq.AppendCallback([WeakThis]()
    {
        AJuiceChestActor* Self = WeakThis.Get();
        if (!Self)
        {
            return;
        }

        if (Self->LidHinge)
        {
            UTween::ComponentRelativeRotation(Self->LidHinge, FRotator(-115.f, 0.f, 0.f), 0.75f)
                .SetEase(ETweenEase::OutBounce);
        }

        const FVector Offsets[] = {
            FVector(-15.f, -35.f, 75.f),
            FVector(15.f,  -15.f, 105.f),
            FVector(-15.f,  20.f, 115.f),
            FVector(15.f,   40.f, 75.f)
        };

        Self->LootSpinHandles.Reset();

        for (int32 i = 0; i < Self->LootItems.Num(); ++i)
        {
            if (UStaticMeshComponent* Item = Self->LootItems[i])
            {
                const float Delay = 0.25f + static_cast<float>(i) * 0.1f;
                const FVector TargetScale = (i % 2 == 0) ? FVector(0.35f) : FVector(0.3f, 0.3f, 0.15f);

                UTween::ComponentRelativeLocation(Item, Offsets[i % 4], 0.6f)
                    .SetDelay(Delay)
                    .SetEase(ETweenEase::OutQuad);

                UTween::ComponentRelativeScale(Item, TargetScale, 0.5f)
                    .SetDelay(Delay)
                    .SetEase(ETweenEase::OutBack);

                FTweenHandle Spin = UTween::ComponentRelativeRotation(Item, FRotator(0.f, 360.f, 0.f), 2.2f)
                    .SetDelay(Delay)
                    .SetEase(ETweenEase::Linear)
                    .SetLoops(-1, ETweenLoopMode::Restart);

                Self->LootSpinHandles.Add(Spin);
            }
        }
    });

    Seq.AppendInterval(0.9f);
    Seq.OnComplete([WeakThis]()
    {
        if (AJuiceChestActor* Self = WeakThis.Get())
        {
            Self->bIsOpen = true;
            Self->bIsAnimating = false;
            Self->OnChestStateChanged.Broadcast(true);
        }
    });

    ChestSequenceHandle = Seq.Play();
}

void AJuiceChestActor::CloseChest()
{
    if (bIsAnimating || !bIsOpen)
    {
        return;
    }

    bIsAnimating = true;
    ChestSequenceHandle.Kill();

    for (FTweenHandle& Handle : LootSpinHandles)
    {
        Handle.Kill();
    }
    LootSpinHandles.Reset();

    for (int32 i = 0; i < LootItems.Num(); ++i)
    {
        if (UStaticMeshComponent* Item = LootItems[i])
        {
            UTween::ComponentRelativeLocation(Item, FVector::ZeroVector, 0.28f)
                .SetEase(ETweenEase::InQuad);
            UTween::ComponentRelativeScale(Item, FVector::ZeroVector, 0.28f)
                .SetEase(ETweenEase::InBack);
        }
    }

    TWeakObjectPtr<AJuiceChestActor> WeakThis(this);
    FTweenSequenceHandle Seq = UTween::Sequence().SetBoundTarget(this);
    Seq.AppendInterval(0.25f);

    if (LidHinge)
    {
        Seq.Append(UTween::ComponentRelativeRotation(LidHinge, FRotator::ZeroRotator, 0.3f)
            .SetEase(ETweenEase::InQuad));
    }

    Seq.OnComplete([WeakThis]()
    {
        if (AJuiceChestActor* Self = WeakThis.Get())
        {
            UTween::PunchPosition(Self, FVector(0.f, 0.f, -8.f), 0.25f, 10);

            if (Self->LockDynMat)
            {
                UTween::MaterialVector(Self->LockDynMat, FName("Color"), FLinearColor(1.0f, 0.8f, 0.1f, 1.0f), 0.2f);
            }

            Self->bIsOpen = false;
            Self->bIsAnimating = false;
            Self->OnChestStateChanged.Broadcast(false);
        }
    });

    ChestSequenceHandle = Seq.Play();
}
