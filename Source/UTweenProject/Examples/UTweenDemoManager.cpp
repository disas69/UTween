#include "UTweenDemoManager.h"

#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

#include "UTween.h"
#include "UTweenMacros.h"

AUTweenDemoManager::AUTweenDemoManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeFinder.Succeeded())
    {
        ShapeMesh = CubeFinder.Object;
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (MaterialFinder.Succeeded())
    {
        ShapeMaterial = MaterialFinder.Object;
    }
}

void AUTweenDemoManager::BeginPlay()
{
    Super::BeginPlay();

    if (!ShapeMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("AUTweenDemoManager: ShapeMesh not set, aborting demo."));
        return;
    }

    SpawnedActors.Reset();
    SpawnedActors.Reserve(NumShapes);

    for (int32 i = 0; i < NumShapes; ++i)
    {
        if (AStaticMeshActor* Shape = SpawnShape(i))
        {
            SpawnedActors.Add(Shape);
            StartAnimationFor(i, Shape);
        }
    }
}

AStaticMeshActor* AUTweenDemoManager::SpawnShape(int32 Index)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    const FVector Dir = LineDirection.IsNearlyZero() ? FVector(0.f, 1.f, 0.f) : LineDirection.GetSafeNormal();
    const FVector Location = GetActorLocation() + Dir * (Spacing * static_cast<float>(Index));
    const FTransform SpawnTransform(FRotator::ZeroRotator, Location);

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AStaticMeshActor* Shape = World->SpawnActorDeferred<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(), SpawnTransform, this);
    if (!Shape)
    {
        return nullptr;
    }

    Shape->SetMobility(EComponentMobility::Movable);

    if (UStaticMeshComponent* SMC = Shape->GetStaticMeshComponent())
    {
        SMC->SetStaticMesh(ShapeMesh);
        if (ShapeMaterial)
        {
            SMC->SetMaterial(0, ShapeMaterial);
        }
        SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

#if WITH_EDITOR
    Shape->SetActorLabel(FString::Printf(TEXT("UTweenDemoShape_%02d"), Index));
#endif

    Shape->FinishSpawning(SpawnTransform);
    return Shape;
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

// 0 — Vertical bounce: UTWEEN_POSITION + yoyo + OutBounce.
void AUTweenDemoManager::Demo0_VerticalBounce(AStaticMeshActor* Shape, const FVector& Base)
{
    UTWEEN_POSITION(Shape, Base, Base + FVector(0.f, 0.f, 200.f), 1.2f)
        .SetEase(ETweenEase::OutBounce)
        .SetLoops(-1, ETweenLoopMode::Yoyo);
}

// 1 — Continuous yaw spin: UTWEEN_ROTATION + infinite yoyo.
void AUTweenDemoManager::Demo1_ContinuousSpin(AStaticMeshActor* Shape)
{
    UTWEEN_ROTATION(Shape, FRotator::ZeroRotator, FRotator(0.f, 360.f, 0.f), 2.0f)
        .SetEase(ETweenEase::InOutQuad)
        .SetLoops(-1, ETweenLoopMode::Yoyo);
}

// 2 — Scale pulse: UTWEEN_SCALE + yoyo + InOutSine.
void AUTweenDemoManager::Demo2_ScalePulse(AStaticMeshActor* Shape)
{
    UTWEEN_SCALE(Shape, FVector(1.f), FVector(1.5f), 0.8f)
        .SetEase(ETweenEase::InOutSine)
        .SetLoops(-1, ETweenLoopMode::Yoyo);
}

// 3 — 4-step slide: UTWEEN_SEQUENCE with four UTWEEN_POSITION legs.
void AUTweenDemoManager::Demo3_SlideSequence(AStaticMeshActor* Shape, const FVector& Base)
{
    const FVector Right = Base + FVector(150.f, 0.f, 0.f);
    const FVector Up    = Base + FVector(0.f, 0.f, 200.f);
    const FVector Left  = Base + FVector(-150.f, 0.f, 0.f);

    UTWEEN_SEQUENCE(Shape,
        UTWEEN_POSITION(Shape, Base,  Right, 0.5f).SetEase(ETweenEase::OutCubic),
        UTWEEN_POSITION(Shape, Right, Up,    0.5f).SetEase(ETweenEase::OutCubic),
        UTWEEN_POSITION(Shape, Up,    Left,  0.5f).SetEase(ETweenEase::OutCubic),
        UTWEEN_POSITION(Shape, Left,  Base,  0.5f).SetEase(ETweenEase::OutCubic))
        .SetLoops(-1, ETweenLoopMode::Restart)
        .Play();
}

// 4 — Elastic pop: UTWEEN_SCALE + yoyo + OutElastic.
void AUTweenDemoManager::Demo4_ElasticPop(AStaticMeshActor* Shape)
{
    UTWEEN_SCALE(Shape, FVector(0.f), FVector(1.f), 1.0f)
        .SetEase(ETweenEase::OutElastic)
        .SetLoops(-1, ETweenLoopMode::Yoyo);
}

// 5 — Barrel roll on pitch: UTWEEN_ROTATION + infinite restart.
void AUTweenDemoManager::Demo5_BarrelRoll(AStaticMeshActor* Shape)
{
    UTWEEN_ROTATION(Shape, FRotator::ZeroRotator, FRotator(0.f, 0.f, 360.f), 1.5f)
        .SetEase(ETweenEase::InOutCubic)
        .SetLoops(-1, ETweenLoopMode::Restart);
}

// 6 — Parametric Lissajous orbit: UTWEEN (generic value) driving SetActorLocation via a lambda.
void AUTweenDemoManager::Demo6_Orbit(AStaticMeshActor* Shape, const FVector& Base)
{
    TWeakObjectPtr<AStaticMeshActor> WeakShape(Shape);
    UTWEEN(Shape, 0.f, 2.f * PI,
        ([WeakShape, Base](float Theta)
        {
            if (AStaticMeshActor* ValidShape = WeakShape.Get())
            {
                const FVector Offset(
                    80.f * FMath::Sin(Theta),
                    0.f,
                    60.f * FMath::Sin(2.f * Theta));
                ValidShape->SetActorLocation(Base + Offset);
            }
        }),
        3.f)
        .SetEase(ETweenEase::Linear)
        .SetLoops(-1, ETweenLoopMode::Restart);
}

// 7 — Jump + flip: UTWEEN_PARALLEL with a delayed return tween to overlap with a continuous spin.
void AUTweenDemoManager::Demo7_JumpFlip(AStaticMeshActor* Shape, const FVector& Base)
{
    const FVector Apex = Base + FVector(0.f, 0.f, 250.f);

    UTWEEN_PARALLEL(Shape,
        UTWEEN_POSITION(Shape, Base, Apex, 0.6f).SetEase(ETweenEase::OutQuad),
        UTWEEN_POSITION(Shape, Apex, Base, 0.6f).SetEase(ETweenEase::InQuad).SetDelay(0.6f),
        UTWEEN_ROTATION(Shape, FRotator::ZeroRotator, FRotator(720.f, 0.f, 0.f), 1.2f)
            .SetEase(ETweenEase::InOutCubic))
        .SetLoops(-1, ETweenLoopMode::Restart)
        .Play();
}

// 8 — Parallel combo: UTWEEN_PARALLEL of position + rotation + scale, yoyo-looped.
void AUTweenDemoManager::Demo8_ParallelCombo(AStaticMeshActor* Shape, const FVector& Base)
{
    const FVector High = Base + FVector(0.f, 0.f, 180.f);

    UTWEEN_PARALLEL(Shape,
        UTWEEN_POSITION(Shape, Base, High, 1.2f).SetEase(ETweenEase::InOutQuad),
        UTWEEN_ROTATION(Shape, FRotator::ZeroRotator, FRotator(0.f, 360.f, 180.f), 1.2f)
            .SetEase(ETweenEase::InOutQuad),
        UTWEEN_SCALE(Shape, FVector(0.8f), FVector(1.6f), 1.2f).SetEase(ETweenEase::InOutSine))
        .SetLoops(-1, ETweenLoopMode::Yoyo)
        .Play();
}

// 9 — Nested shake: outer UTWEEN_SEQUENCE fires a sub-sequence via UTWEEN_DO (fire-and-forget).
void AUTweenDemoManager::Demo9_NestedShake(AStaticMeshActor* Shape, const FVector& Base)
{
    FTweenSequenceHandle Shake = UTWEEN_SEQUENCE(Shape,
        UTWEEN_ROTATION(Shape, FRotator::ZeroRotator,   FRotator(0.f,  20.f, 0.f), 0.08f).SetEase(ETweenEase::InOutSine),
        UTWEEN_ROTATION(Shape, FRotator(0.f, 20.f, 0.f), FRotator(0.f, -20.f, 0.f), 0.16f).SetEase(ETweenEase::InOutSine),
        UTWEEN_ROTATION(Shape, FRotator(0.f,-20.f, 0.f), FRotator::ZeroRotator,     0.08f).SetEase(ETweenEase::InOutSine))
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
