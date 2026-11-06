#include "UTweenMacrosExample.h"

#include "Blueprint/UserWidget.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AUTweenMacrosExample::AUTweenMacrosExample()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;
}

void AUTweenMacrosExample::BeginPlay()
{
    Super::BeginPlay();
    RunAllDemos();
}

void AUTweenMacrosExample::RunAllDemos()
{
    DemoGenericValue();
    DemoTargetSpecific();
    DemoSequence();
    DemoParallel();
    DemoNested();
}

void AUTweenMacrosExample::OnAlphaUpdate(float NewAlpha)
{
    CurrentAlpha = NewAlpha;
}

void AUTweenMacrosExample::OnOffsetUpdate(const FVector& NewOffset)
{
    CurrentOffset = NewOffset;
}

void AUTweenMacrosExample::DemoGenericValue()
{
    // Member function pointer form.
    UTWEEN(this, 0.f, 100.f, &AUTweenMacrosExample::OnAlphaUpdate, 1.f)
        .SetEase(ETweenEase::OutQuad)
        .SetDelay(0.25f);

    // Lambda form.
    TWeakObjectPtr<AUTweenMacrosExample> Weak(this);
    UTWEEN(this, FVector::ZeroVector, FVector(0.f, 0.f, 200.f),
        [Weak](const FVector& V)
        {
            if (AUTweenMacrosExample* Self = Weak.Get())
            {
                Self->OnOffsetUpdate(V);
            }
        },
        1.f)
        .SetEase(ETweenEase::InOutCubic)
        .OnComplete([Weak]
            {
                if (AUTweenMacrosExample* Self = Weak.Get())
                {
                    UE_LOG(LogTemp, Log, TEXT("UTWEEN lambda finished. Final offset: %s"), *Self->CurrentOffset.ToString());
                }
            });
}

void AUTweenMacrosExample::DemoTargetSpecific()
{
    const FVector Start = GetActorLocation();
    const FVector End = Start + FVector(0.f, 0.f, 150.f);
    UTWEEN_POSITION(this, Start, End, 0.5f).SetEase(ETweenEase::InOutCubic);

    if (Root)
    {
        UTWEEN_SCALE(Root, FVector(1.f), FVector(1.5f), 0.3f)
            .SetLoops(2, ETweenLoopMode::Yoyo);
    }

    UTWEEN_ROTATION(this, FRotator::ZeroRotator, FRotator(0.f, 180.f, 0.f), 0.5f);

    if (DemoWidget)
    {
        UTWEEN_POSITION(DemoWidget, FVector2D::ZeroVector, FVector2D(100.f, 0.f), 0.3f);
        UTWEEN_SCALE(DemoWidget, FVector2D(0.9f), FVector2D(1.f), 0.3f);
        UTWEEN_OPACITY(DemoWidget, 0.f, 1.f, 0.2f);
        UTWEEN_COLOR(DemoWidget, FLinearColor::Black, FLinearColor::White, 0.4f);
    }
}

void AUTweenMacrosExample::DemoSequence()
{
    UTWEEN_SEQUENCE(this,
        UTWEEN(this, 0.f, 100.f, &AUTweenMacrosExample::OnAlphaUpdate, 0.5f),
        UTWEEN_WAIT(0.25f),
        UTWEEN_DO([] { UE_LOG(LogTemp, Log, TEXT("UTWEEN_SEQUENCE: halfway")); }),
        UTWEEN(this, 100.f, 0.f, &AUTweenMacrosExample::OnAlphaUpdate, 0.5f))
        .SetLoops(2, ETweenLoopMode::Yoyo)
        .OnComplete([] { UE_LOG(LogTemp, Log, TEXT("UTWEEN_SEQUENCE: done")); })
        .Play();
}

void AUTweenMacrosExample::DemoParallel()
{
    if (!DemoWidget)
    {
        return;
    }

    UTWEEN_PARALLEL(this,
        UTWEEN_POSITION(DemoWidget, FVector2D::ZeroVector, FVector2D(200.f, 0.f), 0.4f),
        UTWEEN_OPACITY(DemoWidget, 0.f, 1.f, 0.4f),
        UTWEEN_SCALE(DemoWidget, FVector2D(0.9f), FVector2D(1.f), 0.4f))
        .Play();
}

// Sub-sequences are fire-and-forget; the parent does not wait for them.
void AUTweenMacrosExample::DemoNested()
{
    if (!DemoWidget)
    {
        return;
    }

    UTWEEN_SEQUENCE(this,
        UTWEEN_PARALLEL(this,
            UTWEEN_OPACITY(DemoWidget, 0.f, 1.f, 0.2f),
            UTWEEN_SCALE(DemoWidget, FVector2D(0.8f), FVector2D(1.f), 0.2f)),
        UTWEEN_WAIT(1.f),
        UTWEEN_OPACITY(DemoWidget, 1.f, 0.f, 0.15f))
        .Play();
}
