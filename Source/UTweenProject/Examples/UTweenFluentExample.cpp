#include "UTweenFluentExample.h"

#include "Blueprint/UserWidget.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AUTweenFluentExample::AUTweenFluentExample()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;
}

void AUTweenFluentExample::BeginPlay()
{
    Super::BeginPlay();
    RunAllDemos();
}

void AUTweenFluentExample::RunAllDemos()
{
    DemoGenericValue();
    DemoTargetSpecific();
    DemoSequence();
    DemoParallel();
    DemoNested();
}

void AUTweenFluentExample::OnAlphaUpdate(float NewAlpha)
{
    CurrentAlpha = NewAlpha;
}

void AUTweenFluentExample::OnOffsetUpdate(const FVector& NewOffset)
{
    CurrentOffset = NewOffset;
}

void AUTweenFluentExample::DemoGenericValue()
{
    TWeakObjectPtr<AUTweenFluentExample> Weak(this);

    UTween::Value(0.f, 100.f, 1.f,
        [Weak](float V)
        {
            if (AUTweenFluentExample* Self = Weak.Get())
            {
                Self->OnAlphaUpdate(V);
            }
        })
        .SetEase(ETweenEase::OutQuad)
        .SetDelay(0.25f);

    UTween::Value(FVector::ZeroVector, FVector(0.f, 0.f, 200.f), 1.f,
        [Weak](const FVector& V)
        {
            if (AUTweenFluentExample* Self = Weak.Get())
            {
                Self->OnOffsetUpdate(V);
            }
        })
        .SetEase(ETweenEase::InOutCubic)
        .OnComplete([Weak]
            {
                if (AUTweenFluentExample* Self = Weak.Get())
                {
                    UE_LOG(LogTemp, Log, TEXT("UTween::Value lambda finished. Final offset: %s"),
                        *Self->CurrentOffset.ToString());
                }
            });
}

void AUTweenFluentExample::DemoTargetSpecific()
{
    const FVector Start = GetActorLocation();
    const FVector End = Start + FVector(0.f, 0.f, 150.f);
    UTween::ActorLocation(this, End, 0.5f)
        .From(Start)
        .SetEase(ETweenEase::InOutCubic);

    if (Root)
    {
        UTween::ComponentRelativeScale(Root, FVector(1.5f), 0.3f)
            .From(FVector(1.f))
            .SetLoops(2, ETweenLoopMode::Yoyo);
    }

    UTween::ActorRotation(this, FRotator(0.f, 180.f, 0.f), 0.5f)
        .From(FRotator::ZeroRotator);

    if (DemoWidget)
    {
        UTween::WidgetPosition(DemoWidget, FVector2D(100.f, 0.f), 0.3f)
            .From(FVector2D::ZeroVector);

        UTween::WidgetScale(DemoWidget, FVector2D(1.f), 0.3f)
            .From(FVector2D(0.9f));

        UTween::WidgetOpacity(DemoWidget, 1.f, 0.2f)
            .From(0.f);

        UTween::WidgetColor(DemoWidget, FLinearColor::White, 0.4f)
            .From(FLinearColor::Black);
    }
}

void AUTweenFluentExample::DemoSequence()
{
    TWeakObjectPtr<AUTweenFluentExample> Weak(this);

    UTween::Sequence()
        .SetBoundTarget(this)
        .SetTag(FTweenTag(GetFName()))
        .Append(UTween::Value(0.f, 100.f, 0.5f,
            [Weak](float V)
            {
                if (AUTweenFluentExample* Self = Weak.Get())
                {
                    Self->OnAlphaUpdate(V);
                }
            }))
        .AppendInterval(0.25f)
        .AppendCallback([] { UE_LOG(LogTemp, Log, TEXT("Fluent sequence: halfway")); })
        .Append(UTween::Value(100.f, 0.f, 0.5f,
            [Weak](float V)
            {
                if (AUTweenFluentExample* Self = Weak.Get())
                {
                    Self->OnAlphaUpdate(V);
                }
            }))
        .SetLoops(2, ETweenLoopMode::Yoyo)
        .OnComplete([] { UE_LOG(LogTemp, Log, TEXT("Fluent sequence: done")); })
        .Play();
}

void AUTweenFluentExample::DemoParallel()
{
    if (!DemoWidget)
    {
        return;
    }

    UTween::Sequence()
        .SetBoundTarget(this)
        .Append(UTween::WidgetPosition(DemoWidget, FVector2D(200.f, 0.f), 0.4f)
            .From(FVector2D::ZeroVector))
        .Join(UTween::WidgetOpacity(DemoWidget, 1.f, 0.4f)
            .From(0.f))
        .Join(UTween::WidgetScale(DemoWidget, FVector2D(1.f), 0.4f)
            .From(FVector2D(0.9f)))
        .Play();
}

// Inner sequence is fire-and-forget via a callback step.
void AUTweenFluentExample::DemoNested()
{
    if (!DemoWidget)
    {
        return;
    }

    FTweenSequenceHandle Inner = UTween::Sequence()
        .SetBoundTarget(this)
        .Append(UTween::WidgetOpacity(DemoWidget, 1.f, 0.2f).From(0.f))
        .Join(UTween::WidgetScale(DemoWidget, FVector2D(1.f), 0.2f).From(FVector2D(0.8f)));

    UTween::Sequence()
        .SetBoundTarget(this)
        .AppendCallback([Inner]() mutable { Inner.Play(); })
        .AppendInterval(1.f)
        .Append(UTween::WidgetOpacity(DemoWidget, 0.f, 0.15f).From(1.f))
        .Play();
}
