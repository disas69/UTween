#include "UTween.h"
#include "UTweenModule.h"
#include "UTweenSubsystem.h"
#include "TweenInstance.h"
#include "TweenSequence.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/UObjectIterator.h"

UTweenSubsystem* UTween::GetSubsystem(const UObject* WorldContext)
{
    if (WorldContext)
    {
        if (const UTweenSubsystem* DirectSub = Cast<UTweenSubsystem>(WorldContext))
        {
            return const_cast<UTweenSubsystem*>(DirectSub);
        }

        const UGameInstance* TargetGI = Cast<UGameInstance>(WorldContext);
        if (!TargetGI)
        {
            if (const UWorld* DirectWorld = WorldContext->GetWorld())
            {
                TargetGI = DirectWorld->GetGameInstance();
            }
            else if (GEngine)
            {
                if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull))
                {
                    TargetGI = World->GetGameInstance();
                }
            }
        }
        if (!TargetGI)
        {
            TargetGI = WorldContext->GetTypedOuter<UGameInstance>();
        }

        if (TargetGI)
        {
            if (UTweenSubsystem* Sub = TargetGI->GetSubsystem<UTweenSubsystem>())
            {
                return Sub;
            }
            for (TObjectIterator<UTweenSubsystem> It; It; ++It)
            {
                if (It->GetOuter() == TargetGI || It->GetGameInstance() == TargetGI)
                {
                    return *It;
                }
            }
        }

        if (UTweenSubsystem* Sub = UTweenSubsystem::Get(WorldContext))
        {
            return Sub;
        }
    }

    if (!GEngine)
    {
        TObjectIterator<UTweenSubsystem> FallbackIt;
        if (FallbackIt)
        {
            return *FallbackIt;
        }
        return nullptr;
    }

    if (GEngine->GameViewport)
    {
        if (const UWorld* World = GEngine->GameViewport->GetWorld())
        {
            if (UGameInstance* GI = World->GetGameInstance())
            {
                if (UTweenSubsystem* Sub = GI->GetSubsystem<UTweenSubsystem>())
                {
                    return Sub;
                }
            }
        }
    }

    // Walk world contexts (covers PIE, standalone, dedicated server).
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World();
        if (!World)
        {
            continue;
        }
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UTweenSubsystem* Sub = GI->GetSubsystem<UTweenSubsystem>())
            {
                return Sub;
            }
        }
    }

    TObjectIterator<UTweenSubsystem> FinalFallbackIt;
    if (FinalFallbackIt)
    {
        return *FinalFallbackIt;
    }

    return nullptr;
}

template <typename T>
FTweenHandle UTween::CreateValueTween(
    const UObject* WorldContext,
    const T& From, const T& To, float Duration,
    TFunction<void(const T&)> OnUpdate,
    const TCHAR* MethodName)
{
    UTweenSubsystem* Subsystem = GetSubsystem(WorldContext);
    if (!Subsystem)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::%s called with no active subsystem; returning invalid handle."), MethodName);
        return FTweenHandle();
    }

    TSharedPtr<TTweenInstance<T>> Instance = MakeShared<TTweenInstance<T>>();
    Instance->StartValue = From;
    Instance->EndValue = To;
    Instance->Duration = FMath::Max(0.f, Duration);
    Instance->bUseFrom = true;
    Instance->DebugName = FName(MethodName);
    Instance->LerpFunc = [](const T& A, const T& B, float Alpha)
    {
        return FMath::Lerp(A, B, Alpha);
    };
    Instance->ApplyFunc = [Cb = MoveTemp(OnUpdate)](const T& Val)
    {
        if (Cb)
        {
            Cb(Val);
        }
    };

    Subsystem->RegisterTween(Instance);
    UE_LOG(LogUTween, Verbose, TEXT("UTween::%s created (Id=%lld)"), MethodName, Instance->Id.Value);
    return FTweenHandle(Instance);
}

FTweenHandle UTween::Value(float From, float To, float Duration,
    TFunction<void(float)> OnUpdate)
{
    return Value(nullptr, From, To, Duration, MoveTemp(OnUpdate));
}

FTweenHandle UTween::Value(const UObject* WorldContext, float From, float To, float Duration,
    TFunction<void(float)> OnUpdate)
{
    // Adapt scalar callback to the (const T&) signature used by CreateValueTween.
    TFunction<void(const float&)> Wrapped = [Cb = MoveTemp(OnUpdate)](const float& V)
    {
        if (Cb)
        {
            Cb(V);
        }
    };
    return CreateValueTween<float>(WorldContext, From, To, Duration, MoveTemp(Wrapped), TEXT("Value<float>"));
}

FTweenHandle UTween::Value(const FVector2D& From, const FVector2D& To, float Duration,
    TFunction<void(const FVector2D&)> OnUpdate)
{
    return Value(nullptr, From, To, Duration, MoveTemp(OnUpdate));
}

FTweenHandle UTween::Value(const UObject* WorldContext, const FVector2D& From, const FVector2D& To, float Duration,
    TFunction<void(const FVector2D&)> OnUpdate)
{
    return CreateValueTween<FVector2D>(WorldContext, From, To, Duration, MoveTemp(OnUpdate), TEXT("Value<FVector2D>"));
}

FTweenHandle UTween::Value(const FVector& From, const FVector& To, float Duration,
    TFunction<void(const FVector&)> OnUpdate)
{
    return Value(nullptr, From, To, Duration, MoveTemp(OnUpdate));
}

FTweenHandle UTween::Value(const UObject* WorldContext, const FVector& From, const FVector& To, float Duration,
    TFunction<void(const FVector&)> OnUpdate)
{
    return CreateValueTween<FVector>(WorldContext, From, To, Duration, MoveTemp(OnUpdate), TEXT("Value<FVector>"));
}

FTweenHandle UTween::Value(const FRotator& From, const FRotator& To, float Duration,
    TFunction<void(const FRotator&)> OnUpdate)
{
    return Value(nullptr, From, To, Duration, MoveTemp(OnUpdate));
}

FTweenHandle UTween::Value(const UObject* WorldContext, const FRotator& From, const FRotator& To, float Duration,
    TFunction<void(const FRotator&)> OnUpdate)
{
    return CreateValueTween<FRotator>(WorldContext, From, To, Duration, MoveTemp(OnUpdate), TEXT("Value<FRotator>"));
}

FTweenHandle UTween::Value(const FLinearColor& From, const FLinearColor& To, float Duration,
    TFunction<void(const FLinearColor&)> OnUpdate)
{
    return Value(nullptr, From, To, Duration, MoveTemp(OnUpdate));
}

FTweenHandle UTween::Value(const UObject* WorldContext, const FLinearColor& From, const FLinearColor& To, float Duration,
    TFunction<void(const FLinearColor&)> OnUpdate)
{
    return CreateValueTween<FLinearColor>(WorldContext, From, To, Duration, MoveTemp(OnUpdate), TEXT("Value<FLinearColor>"));
}

template <typename TValue>
FTweenHandle UTween::CreateTargetTween(
    UObject* Target,
    const TValue& To,
    float Duration,
    TFunction<TValue()> Getter,
    TFunction<void(const TValue&)> Apply,
    TFunction<TValue(const TValue&, const TValue&, float)> Lerp,
    const TCHAR* MethodName)
{
    if (!Target)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::%s called with null target; returning invalid handle."), MethodName);
        return FTweenHandle();
    }

    UTweenSubsystem* Subsystem = GetSubsystem(Target);
    if (!Subsystem)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::%s called with no active subsystem; returning invalid handle."), MethodName);
        return FTweenHandle();
    }

    TSharedPtr<TTweenInstance<TValue>> Instance = MakeShared<TTweenInstance<TValue>>();
    Instance->EndValue = To;
    Instance->Duration = FMath::Max(0.f, Duration);
    Instance->BoundTarget = Target;
    Instance->bHasTarget = (Target != nullptr);
    Instance->DebugName = FName(MethodName);
    Instance->GetterFunc = MoveTemp(Getter);
    Instance->ApplyFunc = MoveTemp(Apply);

    if (Lerp)
    {
        Instance->LerpFunc = MoveTemp(Lerp);
    }
    else
    {
        Instance->LerpFunc = [](const TValue& A, const TValue& B, float Alpha)
        {
            return FMath::Lerp(A, B, Alpha);
        };
    }

    Subsystem->RegisterTween(Instance);
    UE_LOG(LogUTween, Verbose, TEXT("UTween::%s created (Id=%lld)"), MethodName, Instance->Id.Value);
    return FTweenHandle(Instance);
}

FTweenHandle UTween::WidgetPosition(UWidget* Target, const FVector2D& To, float Duration)
{
    TWeakObjectPtr<UWidget> Weak(Target);
    return CreateTargetTween<FVector2D>(
        Target, To, Duration,
        [Weak]() -> FVector2D
        {
            if (UWidget* W = Weak.Get())
            {
                return W->GetRenderTransform().Translation;
            }
            return FVector2D::ZeroVector;
        },
        [Weak](const FVector2D& V)
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderTranslation(V);
            }
        },
        nullptr,
        TEXT("WidgetPosition"));
}

FTweenHandle UTween::WidgetScale(UWidget* Target, const FVector2D& To, float Duration)
{
    TWeakObjectPtr<UWidget> Weak(Target);
    return CreateTargetTween<FVector2D>(
        Target, To, Duration,
        [Weak]() -> FVector2D
        {
            if (UWidget* W = Weak.Get())
            {
                return W->GetRenderTransform().Scale;
            }
            return FVector2D::UnitVector;
        },
        [Weak](const FVector2D& V)
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderScale(V);
            }
        },
        nullptr,
        TEXT("WidgetScale"));
}

FTweenHandle UTween::WidgetRotation(UWidget* Target, float To, float Duration)
{
    TWeakObjectPtr<UWidget> Weak(Target);
    return CreateTargetTween<float>(
        Target, To, Duration,
        [Weak]() -> float
        {
            if (UWidget* W = Weak.Get())
            {
                return W->GetRenderTransform().Angle;
            }
            return 0.f;
        },
        [Weak](const float& V)
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderTransformAngle(V);
            }
        },
        nullptr,
        TEXT("WidgetRotation"));
}

FTweenHandle UTween::WidgetOpacity(UWidget* Target, float To, float Duration)
{
    TWeakObjectPtr<UWidget> Weak(Target);
    return CreateTargetTween<float>(
        Target, To, Duration,
        [Weak]() -> float
        {
            if (UWidget* W = Weak.Get())
            {
                return W->GetRenderOpacity();
            }
            return 1.f;
        },
        [Weak](const float& V)
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderOpacity(V);
            }
        },
        nullptr,
        TEXT("WidgetOpacity"));
}

FTweenHandle UTween::CanvasPosition(UWidget* Target, const FVector2D& To, float Duration)
{
    if (!Target)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::CanvasPosition called with null target; returning invalid handle."));
        return FTweenHandle();
    }

    UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Target->Slot);
    if (!CanvasSlot)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::CanvasPosition: Target widget slot is not a UCanvasPanelSlot; returning invalid handle."));
        return FTweenHandle();
    }

    return CanvasPosition(CanvasSlot, To, Duration);
}

FTweenHandle UTween::CanvasPosition(UCanvasPanelSlot* Target, const FVector2D& To, float Duration)
{
    TWeakObjectPtr<UCanvasPanelSlot> Weak(Target);
    return CreateTargetTween<FVector2D>(
        Target, To, Duration,
        [Weak]() -> FVector2D
        {
            if (UCanvasPanelSlot* Slot = Weak.Get())
            {
                return Slot->GetPosition();
            }
            return FVector2D::ZeroVector;
        },
        [Weak](const FVector2D& V)
        {
            if (UCanvasPanelSlot* Slot = Weak.Get())
            {
                Slot->SetPosition(V);
            }
        },
        nullptr,
        TEXT("CanvasPosition"));
}

FTweenHandle UTween::CanvasSize(UWidget* Target, const FVector2D& To, float Duration)
{
    if (!Target)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::CanvasSize called with null target; returning invalid handle."));
        return FTweenHandle();
    }

    UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Target->Slot);
    if (!CanvasSlot)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::CanvasSize: Target widget slot is not a UCanvasPanelSlot; returning invalid handle."));
        return FTweenHandle();
    }

    return CanvasSize(CanvasSlot, To, Duration);
}

FTweenHandle UTween::CanvasSize(UCanvasPanelSlot* Target, const FVector2D& To, float Duration)
{
    TWeakObjectPtr<UCanvasPanelSlot> Weak(Target);
    return CreateTargetTween<FVector2D>(
        Target, To, Duration,
        [Weak]() -> FVector2D
        {
            if (UCanvasPanelSlot* Slot = Weak.Get())
            {
                return Slot->GetSize();
            }
            return FVector2D::ZeroVector;
        },
        [Weak](const FVector2D& V)
        {
            if (UCanvasPanelSlot* Slot = Weak.Get())
            {
                Slot->SetSize(V);
            }
        },
        nullptr,
        TEXT("CanvasSize"));
}

FTweenHandle UTween::WidgetColor(UWidget* Target, const FLinearColor& To, float Duration)
{
    if (!Target)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::WidgetColor called with null target; returning invalid handle."));
        return FTweenHandle();
    }

    if (UUserWidget* AsUser = Cast<UUserWidget>(Target))
    {
        TWeakObjectPtr<UUserWidget> Weak(AsUser);
        return CreateTargetTween<FLinearColor>(
            AsUser, To, Duration,
            [Weak]() -> FLinearColor
            {
                if (UUserWidget* W = Weak.Get())
                {
                    return W->GetColorAndOpacity();
                }
                return FLinearColor::White;
            },
            [Weak](const FLinearColor& V)
            {
                if (UUserWidget* W = Weak.Get())
                {
                    W->SetColorAndOpacity(V);
                }
            },
            nullptr,
            TEXT("WidgetColor<UserWidget>"));
    }

    if (UImage* AsImage = Cast<UImage>(Target))
    {
        TWeakObjectPtr<UImage> Weak(AsImage);
        return CreateTargetTween<FLinearColor>(
            AsImage, To, Duration,
            [Weak]() -> FLinearColor
            {
                if (UImage* Img = Weak.Get())
                {
                    return Img->GetColorAndOpacity();
                }
                return FLinearColor::White;
            },
            [Weak](const FLinearColor& V)
            {
                if (UImage* Img = Weak.Get())
                {
                    Img->SetColorAndOpacity(V);
                }
            },
            nullptr,
            TEXT("WidgetColor<Image>"));
    }

    if (UBorder* AsBorder = Cast<UBorder>(Target))
    {
        TWeakObjectPtr<UBorder> Weak(AsBorder);
        return CreateTargetTween<FLinearColor>(
            AsBorder, To, Duration,
            [Weak]() -> FLinearColor
            {
                if (UBorder* Border = Weak.Get())
                {
                    return Border->GetBrushColor();
                }
                return FLinearColor::White;
            },
            [Weak](const FLinearColor& V)
            {
                if (UBorder* Border = Weak.Get())
                {
                    Border->SetBrushColor(V);
                }
            },
            nullptr,
            TEXT("WidgetColor<Border>"));
    }

    if (UTextBlock* AsText = Cast<UTextBlock>(Target))
    {
        TWeakObjectPtr<UTextBlock> Weak(AsText);
        return CreateTargetTween<FLinearColor>(
            AsText, To, Duration,
            [Weak]() -> FLinearColor
            {
                if (UTextBlock* Text = Weak.Get())
                {
                    return Text->GetColorAndOpacity().GetSpecifiedColor();
                }
                return FLinearColor::White;
            },
            [Weak](const FLinearColor& V)
            {
                if (UTextBlock* Text = Weak.Get())
                {
                    Text->SetColorAndOpacity(FSlateColor(V));
                }
            },
            nullptr,
            TEXT("WidgetColor<TextBlock>"));
    }

    UE_LOG(LogUTween, Warning,
        TEXT("UTween::WidgetColor called on unsupported widget type %s; returning invalid handle."),
        *Target->GetClass()->GetName());
    return FTweenHandle();
}

FTweenHandle UTween::ActorLocation(AActor* Target, const FVector& To, float Duration)
{
    TWeakObjectPtr<AActor> Weak(Target);
    return CreateTargetTween<FVector>(
        Target, To, Duration,
        [Weak]() -> FVector
        {
            if (AActor* A = Weak.Get())
            {
                return A->GetActorLocation();
            }
            return FVector::ZeroVector;
        },
        [Weak](const FVector& V)
        {
            if (AActor* A = Weak.Get())
            {
                A->SetActorLocation(V);
            }
        },
        nullptr,
        TEXT("ActorLocation"));
}

FTweenHandle UTween::ActorRotation(AActor* Target, const FRotator& To, float Duration)
{
    TWeakObjectPtr<AActor> Weak(Target);
    return CreateTargetTween<FRotator>(
        Target, To, Duration,
        [Weak]() -> FRotator
        {
            if (AActor* A = Weak.Get())
            {
                return A->GetActorRotation();
            }
            return FRotator::ZeroRotator;
        },
        [Weak](const FRotator& V)
        {
            if (AActor* A = Weak.Get())
            {
                A->SetActorRotation(V);
            }
        },
        &UTweenLerp::LerpRotator,
        TEXT("ActorRotation"));
}

FTweenHandle UTween::ActorScale(AActor* Target, const FVector& To, float Duration)
{
    TWeakObjectPtr<AActor> Weak(Target);
    return CreateTargetTween<FVector>(
        Target, To, Duration,
        [Weak]() -> FVector
        {
            if (AActor* A = Weak.Get())
            {
                return A->GetActorScale3D();
            }
            return FVector::OneVector;
        },
        [Weak](const FVector& V)
        {
            if (AActor* A = Weak.Get())
            {
                A->SetActorScale3D(V);
            }
        },
        nullptr,
        TEXT("ActorScale"));
}

FTweenHandle UTween::ActorScale(AActor* Target, float UniformScale, float Duration)
{
    return ActorScale(Target, FVector(UniformScale), Duration);
}

FTweenHandle UTween::PunchPosition(AActor* Target, const FVector& Punch, float Duration, int32 Vibrato, float Elasticity)
{
    if (!Target)
    {
        return FTweenHandle();
    }

    TWeakObjectPtr<AActor> Weak(Target);
    const FVector BaseLocation = Target->GetActorLocation();

    return Value(
        Target, 0.0f, 1.0f, Duration,
        [Weak, BaseLocation, Punch, Vibrato, Elasticity](float Alpha)
        {
            if (AActor* A = Weak.Get())
            {
                if (Alpha >= 1.0f)
                {
                    A->SetActorLocation(BaseLocation);
                    return;
                }
                const float Decay = (Elasticity <= 0.0f) ? (1.0f - Alpha) : FMath::Pow(FMath::Max(0.0f, 1.0f - Alpha), Elasticity);
                const float Wave = FMath::Sin(Alpha * Vibrato * UE_PI) * Decay;
                A->SetActorLocation(BaseLocation + Punch * Wave);
            }
        })
        .SetEase(ETweenEase::Linear)
        .OnComplete([Weak, BaseLocation]()
        {
            if (AActor* A = Weak.Get())
            {
                A->SetActorLocation(BaseLocation);
            }
        })
        .OnKill([Weak, BaseLocation]()
        {
            if (AActor* A = Weak.Get())
            {
                A->SetActorLocation(BaseLocation);
            }
        });
}

FTweenHandle UTween::PunchScale(AActor* Target, const FVector& Punch, float Duration, int32 Vibrato, float Elasticity)
{
    if (!Target)
    {
        return FTweenHandle();
    }

    TWeakObjectPtr<AActor> Weak(Target);
    const FVector BaseScale = Target->GetActorScale3D();

    return Value(
        Target, 0.0f, 1.0f, Duration,
        [Weak, BaseScale, Punch, Vibrato, Elasticity](float Alpha)
        {
            if (AActor* A = Weak.Get())
            {
                if (Alpha >= 1.0f)
                {
                    A->SetActorScale3D(BaseScale);
                    return;
                }
                const float Decay = (Elasticity <= 0.0f) ? (1.0f - Alpha) : FMath::Pow(FMath::Max(0.0f, 1.0f - Alpha), Elasticity);
                const float Wave = FMath::Sin(Alpha * Vibrato * UE_PI) * Decay;
                A->SetActorScale3D(BaseScale + Punch * Wave);
            }
        })
        .SetEase(ETweenEase::Linear)
        .OnComplete([Weak, BaseScale]()
        {
            if (AActor* A = Weak.Get())
            {
                A->SetActorScale3D(BaseScale);
            }
        })
        .OnKill([Weak, BaseScale]()
        {
            if (AActor* A = Weak.Get())
            {
                A->SetActorScale3D(BaseScale);
            }
        });
}

FTweenHandle UTween::PunchPosition(UWidget* Target, const FVector2D& Punch, float Duration, int32 Vibrato, float Elasticity)
{
    if (!Target)
    {
        return FTweenHandle();
    }

    TWeakObjectPtr<UWidget> Weak(Target);
    const FVector2D BasePos = Target->GetRenderTransform().Translation;

    return Value(
        Target, 0.0f, 1.0f, Duration,
        [Weak, BasePos, Punch, Vibrato, Elasticity](float Alpha)
        {
            if (UWidget* W = Weak.Get())
            {
                if (Alpha >= 1.0f)
                {
                    W->SetRenderTranslation(BasePos);
                    return;
                }
                const float Decay = (Elasticity <= 0.0f) ? (1.0f - Alpha) : FMath::Pow(FMath::Max(0.0f, 1.0f - Alpha), Elasticity);
                const float Wave = FMath::Sin(Alpha * Vibrato * UE_PI) * Decay;
                W->SetRenderTranslation(BasePos + Punch * Wave);
            }
        })
        .SetEase(ETweenEase::Linear)
        .OnComplete([Weak, BasePos]()
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderTranslation(BasePos);
            }
        })
        .OnKill([Weak, BasePos]()
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderTranslation(BasePos);
            }
        });
}

FTweenHandle UTween::PunchScale(UWidget* Target, const FVector2D& Punch, float Duration, int32 Vibrato, float Elasticity)
{
    if (!Target)
    {
        return FTweenHandle();
    }

    TWeakObjectPtr<UWidget> Weak(Target);
    const FVector2D BaseScale = Target->GetRenderTransform().Scale;

    return Value(
        Target, 0.0f, 1.0f, Duration,
        [Weak, BaseScale, Punch, Vibrato, Elasticity](float Alpha)
        {
            if (UWidget* W = Weak.Get())
            {
                if (Alpha >= 1.0f)
                {
                    W->SetRenderScale(BaseScale);
                    return;
                }
                const float Decay = (Elasticity <= 0.0f) ? (1.0f - Alpha) : FMath::Pow(FMath::Max(0.0f, 1.0f - Alpha), Elasticity);
                const float Wave = FMath::Sin(Alpha * Vibrato * UE_PI) * Decay;
                W->SetRenderScale(BaseScale + Punch * Wave);
            }
        })
        .SetEase(ETweenEase::Linear)
        .OnComplete([Weak, BaseScale]()
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderScale(BaseScale);
            }
        })
        .OnKill([Weak, BaseScale]()
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderScale(BaseScale);
            }
        });
}

FTweenHandle UTween::ShakePosition(AActor* Target, float Strength, float Duration, int32 Vibrato, float Randomness)
{
    if (!Target)
    {
        return FTweenHandle();
    }

    TWeakObjectPtr<AActor> Weak(Target);
    const FVector BaseLocation = Target->GetActorLocation();
    const float Freq = static_cast<float>(FMath::Max(1, Vibrato));

    return Value(
        Target, 0.0f, 1.0f, Duration,
        [Weak, BaseLocation, Strength, Freq](float Alpha)
        {
            if (AActor* A = Weak.Get())
            {
                if (Alpha >= 1.0f)
                {
                    A->SetActorLocation(BaseLocation);
                    return;
                }
                const float Decay = 1.0f - Alpha;
                const float WaveX = FMath::Sin(Alpha * Freq * UE_TWO_PI);
                const float WaveY = FMath::Cos(Alpha * (Freq * 1.37f) * UE_TWO_PI);
                const float WaveZ = FMath::Sin(Alpha * (Freq * 0.73f) * UE_TWO_PI);
                const FVector Offset = FVector(WaveX, WaveY, WaveZ) * (Strength * Decay);
                A->SetActorLocation(BaseLocation + Offset);
            }
        })
        .SetEase(ETweenEase::Linear)
        .OnComplete([Weak, BaseLocation]()
        {
            if (AActor* A = Weak.Get())
            {
                A->SetActorLocation(BaseLocation);
            }
        })
        .OnKill([Weak, BaseLocation]()
        {
            if (AActor* A = Weak.Get())
            {
                A->SetActorLocation(BaseLocation);
            }
        });
}

FTweenHandle UTween::ShakePosition(UWidget* Target, float Strength, float Duration, int32 Vibrato)
{
    if (!Target)
    {
        return FTweenHandle();
    }

    TWeakObjectPtr<UWidget> Weak(Target);
    const FVector2D BasePos = Target->GetRenderTransform().Translation;
    const float Freq = static_cast<float>(FMath::Max(1, Vibrato));

    return Value(
        Target, 0.0f, 1.0f, Duration,
        [Weak, BasePos, Strength, Freq](float Alpha)
        {
            if (UWidget* W = Weak.Get())
            {
                if (Alpha >= 1.0f)
                {
                    W->SetRenderTranslation(BasePos);
                    return;
                }
                const float Decay = 1.0f - Alpha;
                const float WaveX = FMath::Sin(Alpha * Freq * UE_TWO_PI);
                const float WaveY = FMath::Cos(Alpha * (Freq * 1.37f) * UE_TWO_PI);
                const FVector2D Offset = FVector2D(WaveX, WaveY) * (Strength * Decay);
                W->SetRenderTranslation(BasePos + Offset);
            }
        })
        .SetEase(ETweenEase::Linear)
        .OnComplete([Weak, BasePos]()
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderTranslation(BasePos);
            }
        })
        .OnKill([Weak, BasePos]()
        {
            if (UWidget* W = Weak.Get())
            {
                W->SetRenderTranslation(BasePos);
            }
        });
}

FTweenHandle UTween::ComponentRelativeLocation(USceneComponent* Target, const FVector& To, float Duration)
{
    TWeakObjectPtr<USceneComponent> Weak(Target);
    return CreateTargetTween<FVector>(
        Target, To, Duration,
        [Weak]() -> FVector
        {
            if (USceneComponent* C = Weak.Get())
            {
                return C->GetRelativeLocation();
            }
            return FVector::ZeroVector;
        },
        [Weak](const FVector& V)
        {
            if (USceneComponent* C = Weak.Get())
            {
                C->SetRelativeLocation(V);
            }
        },
        nullptr,
        TEXT("ComponentRelativeLocation"));
}

FTweenHandle UTween::ComponentRelativeRotation(USceneComponent* Target, const FRotator& To, float Duration)
{
    TWeakObjectPtr<USceneComponent> Weak(Target);
    return CreateTargetTween<FRotator>(
        Target, To, Duration,
        [Weak]() -> FRotator
        {
            if (USceneComponent* C = Weak.Get())
            {
                return C->GetRelativeRotation();
            }
            return FRotator::ZeroRotator;
        },
        [Weak](const FRotator& V)
        {
            if (USceneComponent* C = Weak.Get())
            {
                C->SetRelativeRotation(V);
            }
        },
        &UTweenLerp::LerpRotator,
        TEXT("ComponentRelativeRotation"));
}

FTweenHandle UTween::ComponentRelativeScale(USceneComponent* Target, const FVector& To, float Duration)
{
    TWeakObjectPtr<USceneComponent> Weak(Target);
    return CreateTargetTween<FVector>(
        Target, To, Duration,
        [Weak]() -> FVector
        {
            if (USceneComponent* C = Weak.Get())
            {
                return C->GetRelativeScale3D();
            }
            return FVector::OneVector;
        },
        [Weak](const FVector& V)
        {
            if (USceneComponent* C = Weak.Get())
            {
                C->SetRelativeScale3D(V);
            }
        },
        nullptr,
        TEXT("ComponentRelativeScale"));
}

FTweenHandle UTween::MaterialScalar(UMaterialInstanceDynamic* Target, FName ParamName, float To, float Duration)
{
    TWeakObjectPtr<UMaterialInstanceDynamic> Weak(Target);
    const FName CapturedName = ParamName;
    return CreateTargetTween<float>(
        Target, To, Duration,
        [Weak, CapturedName]() -> float
        {
            if (UMaterialInstanceDynamic* MID = Weak.Get())
            {
                float Value = 0.f;
                MID->GetScalarParameterValue(CapturedName, Value);
                return Value;
            }
            return 0.f;
        },
        [Weak, CapturedName](const float& V)
        {
            if (UMaterialInstanceDynamic* MID = Weak.Get())
            {
                MID->SetScalarParameterValue(CapturedName, V);
            }
        },
        nullptr,
        TEXT("MaterialScalar"));
}

FTweenHandle UTween::Position(AActor* Target, const FVector& From, const FVector& To, float Duration)
{
    return ActorLocation(Target, To, Duration).From(From);
}

FTweenHandle UTween::Position(USceneComponent* Target, const FVector& From, const FVector& To, float Duration)
{
    return ComponentRelativeLocation(Target, To, Duration).From(From);
}

FTweenHandle UTween::Position(UWidget* Target, const FVector2D& From, const FVector2D& To, float Duration)
{
    return WidgetPosition(Target, To, Duration).From(From);
}

FTweenHandle UTween::Scale(AActor* Target, const FVector& From, const FVector& To, float Duration)
{
    return ActorScale(Target, To, Duration).From(From);
}

FTweenHandle UTween::Scale(USceneComponent* Target, const FVector& From, const FVector& To, float Duration)
{
    return ComponentRelativeScale(Target, To, Duration).From(From);
}

FTweenHandle UTween::Scale(UWidget* Target, const FVector2D& From, const FVector2D& To, float Duration)
{
    return WidgetScale(Target, To, Duration).From(From);
}

FTweenHandle UTween::Rotation(AActor* Target, const FRotator& From, const FRotator& To, float Duration)
{
    return ActorRotation(Target, To, Duration).From(From);
}

FTweenHandle UTween::Rotation(USceneComponent* Target, const FRotator& From, const FRotator& To, float Duration)
{
    return ComponentRelativeRotation(Target, To, Duration).From(From);
}

FTweenHandle UTween::Rotation(UWidget* Target, float From, float To, float Duration)
{
    return WidgetRotation(Target, To, Duration).From(From);
}

FTweenHandle UTween::Opacity(UWidget* Target, float From, float To, float Duration)
{
    return WidgetOpacity(Target, To, Duration).From(From);
}

FTweenHandle UTween::Color(UWidget* Target, const FLinearColor& From, const FLinearColor& To, float Duration)
{
    return WidgetColor(Target, To, Duration).From(From);
}

FTweenSequenceHandle UTween::Sequence(const UObject* WorldContext)
{
    UTweenSubsystem* Subsystem = GetSubsystem(WorldContext);
    if (!Subsystem)
    {
        UE_LOG(LogUTween, Warning, TEXT("UTween::Sequence called with no active subsystem; returning invalid handle."));
        return FTweenSequenceHandle();
    }

    TSharedPtr<FTweenSequence> Seq = MakeShared<FTweenSequence>();
    Subsystem->RegisterSequence(Seq);
    UE_LOG(LogUTween, Verbose, TEXT("UTween::Sequence created (Id=%lld)"), Seq->Id.Value);
    return FTweenSequenceHandle(Seq);
}

FTweenHandle UTween::MaterialVector(UMaterialInstanceDynamic* Target, FName ParamName, const FLinearColor& To, float Duration)
{
    TWeakObjectPtr<UMaterialInstanceDynamic> Weak(Target);
    const FName CapturedName = ParamName;
    return CreateTargetTween<FLinearColor>(
        Target, To, Duration,
        [Weak, CapturedName]() -> FLinearColor
        {
            if (UMaterialInstanceDynamic* MID = Weak.Get())
            {
                FLinearColor Value = FLinearColor::White;
                MID->GetVectorParameterValue(CapturedName, Value);
                return Value;
            }
            return FLinearColor::White;
        },
        [Weak, CapturedName](const FLinearColor& V)
        {
            if (UMaterialInstanceDynamic* MID = Weak.Get())
            {
                MID->SetVectorParameterValue(CapturedName, V);
            }
        },
        nullptr,
        TEXT("MaterialVector"));
}
