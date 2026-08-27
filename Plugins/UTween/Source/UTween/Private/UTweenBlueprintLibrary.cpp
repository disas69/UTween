#include "UTweenBlueprintLibrary.h"

#include "UTween.h"
#include "UTweenSubsystem.h"
#include "UTweenModule.h"

#include "Components/Widget.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
    UTweenSubsystem* ResolveSubsystem(const UObject* WorldContext)
    {
        if (UTweenSubsystem* Sub = UTweenSubsystem::Get(WorldContext))
        {
            return Sub;
        }
        return UTween::GetSubsystem();
    }
}

FTweenHandle FBPTweenHandle::Resolve(const UObject* WorldContext) const
{
    UTweenSubsystem* Sub = ResolveSubsystem(WorldContext);
    if (!Sub)
    {
        return FTweenHandle();
    }
    return Sub->FindTweenById(GetId());
}

bool FBPTweenHandle::IsValid(const UObject* WorldContext) const
{
    return Resolve(WorldContext).IsValid();
}

FTweenSequenceHandle FBPTweenSequenceHandle::Resolve(const UObject* WorldContext) const
{
    UTweenSubsystem* Sub = ResolveSubsystem(WorldContext);
    if (!Sub)
    {
        return FTweenSequenceHandle();
    }
    return Sub->FindSequenceById(GetId());
}

bool FBPTweenSequenceHandle::IsValid(const UObject* WorldContext) const
{
    return Resolve(WorldContext).IsValid();
}

FBPTweenHandle UTweenBlueprintLibrary::TweenWidgetPosition(
    UObject* WorldContextObject, UWidget* Target,
    FVector2D To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::WidgetPosition(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenWidgetScale(
    UObject* WorldContextObject, UWidget* Target,
    FVector2D To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::WidgetScale(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenWidgetRotation(
    UObject* WorldContextObject, UWidget* Target,
    float To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::WidgetRotation(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenWidgetOpacity(
    UObject* WorldContextObject, UWidget* Target,
    float To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::WidgetOpacity(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenWidgetColor(
    UObject* WorldContextObject, UWidget* Target,
    FLinearColor To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::WidgetColor(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenActorLocation(
    UObject* WorldContextObject, AActor* Target,
    FVector To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::ActorLocation(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenActorRotation(
    UObject* WorldContextObject, AActor* Target,
    FRotator To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::ActorRotation(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenActorScale(
    UObject* WorldContextObject, AActor* Target,
    FVector To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::ActorScale(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenComponentRelativeLocation(
    UObject* WorldContextObject, USceneComponent* Target,
    FVector To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::ComponentRelativeLocation(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenComponentRelativeRotation(
    UObject* WorldContextObject, USceneComponent* Target,
    FRotator To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::ComponentRelativeRotation(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenComponentRelativeScale(
    UObject* WorldContextObject, USceneComponent* Target,
    FVector To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::ComponentRelativeScale(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenMaterialScalarParameter(
    UObject* WorldContextObject, UMaterialInstanceDynamic* Target,
    FName ParameterName, float To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::MaterialScalar(Target, ParameterName, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenMaterialVectorParameter(
    UObject* WorldContextObject, UMaterialInstanceDynamic* Target,
    FName ParameterName, FLinearColor To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::MaterialVector(Target, ParameterName, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenFloat(
    UObject* WorldContextObject, float From, float To, float Duration,
    const FOnTweenFloatUpdate& OnUpdate, ETweenEase Ease)
{
    FOnTweenFloatUpdate DelegateCopy = OnUpdate;
    FTweenHandle Handle = UTween::Value(WorldContextObject, From, To, Duration,
        [DelegateCopy](float V)
        {
            DelegateCopy.ExecuteIfBound(V);
        }).SetEase(Ease);
    return FBPTweenHandle(Handle);
}

FBPTweenHandle UTweenBlueprintLibrary::TweenVector(
    UObject* WorldContextObject, FVector From, FVector To, float Duration,
    const FOnTweenVectorUpdate& OnUpdate, ETweenEase Ease)
{
    FOnTweenVectorUpdate DelegateCopy = OnUpdate;
    FTweenHandle Handle = UTween::Value(WorldContextObject, From, To, Duration,
        [DelegateCopy](const FVector& V)
        {
            DelegateCopy.ExecuteIfBound(V);
        }).SetEase(Ease);
    return FBPTweenHandle(Handle);
}

FBPTweenHandle UTweenBlueprintLibrary::TweenColor(
    UObject* WorldContextObject, FLinearColor From, FLinearColor To, float Duration,
    const FOnTweenColorUpdate& OnUpdate, ETweenEase Ease)
{
    FOnTweenColorUpdate DelegateCopy = OnUpdate;
    FTweenHandle Handle = UTween::Value(WorldContextObject, From, To, Duration,
        [DelegateCopy](const FLinearColor& V)
        {
            DelegateCopy.ExecuteIfBound(V);
        }).SetEase(Ease);
    return FBPTweenHandle(Handle);
}

void UTweenBlueprintLibrary::PauseTween(UObject* WorldContextObject, FBPTweenHandle Handle)
{
    Handle.Resolve(WorldContextObject).Pause();
}

void UTweenBlueprintLibrary::ResumeTween(UObject* WorldContextObject, FBPTweenHandle Handle)
{
    Handle.Resolve(WorldContextObject).Resume();
}

void UTweenBlueprintLibrary::KillTween(UObject* WorldContextObject, FBPTweenHandle Handle)
{
    Handle.Resolve(WorldContextObject).Kill();
}

bool UTweenBlueprintLibrary::IsTweenValid(UObject* WorldContextObject, FBPTweenHandle Handle)
{
    return Handle.Resolve(WorldContextObject).IsValid();
}

bool UTweenBlueprintLibrary::IsTweenPlaying(UObject* WorldContextObject, FBPTweenHandle Handle)
{
    return Handle.Resolve(WorldContextObject).IsPlaying();
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::CreateTweenSequence(UObject* WorldContextObject)
{
    return FBPTweenSequenceHandle(UTween::Sequence(WorldContextObject));
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::AppendToSequence(
    UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, FBPTweenHandle Tween)
{
    FTweenSequenceHandle SeqHandle = Sequence.Resolve(WorldContextObject);
    FTweenHandle TweenHandle = Tween.Resolve(WorldContextObject);
    SeqHandle.Append(TweenHandle);
    return Sequence;
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::JoinToSequence(
    UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, FBPTweenHandle Tween)
{
    FTweenSequenceHandle SeqHandle = Sequence.Resolve(WorldContextObject);
    FTweenHandle TweenHandle = Tween.Resolve(WorldContextObject);
    SeqHandle.Join(TweenHandle);
    return Sequence;
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::AppendIntervalToSequence(
    UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, float Duration)
{
    Sequence.Resolve(WorldContextObject).AppendInterval(Duration);
    return Sequence;
}

void UTweenBlueprintLibrary::PlaySequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence)
{
    Sequence.Resolve(WorldContextObject).Play();
}

void UTweenBlueprintLibrary::PauseSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence)
{
    Sequence.Resolve(WorldContextObject).Pause();
}

void UTweenBlueprintLibrary::KillSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence)
{
    Sequence.Resolve(WorldContextObject).Kill();
}

bool UTweenBlueprintLibrary::IsSequenceValid(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence)
{
    return Sequence.Resolve(WorldContextObject).IsValid();
}

bool UTweenBlueprintLibrary::IsSequencePlaying(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence)
{
    return Sequence.Resolve(WorldContextObject).IsPlaying();
}

void UTweenBlueprintLibrary::PauseAllTweens(UObject* WorldContextObject)
{
    if (UTweenSubsystem* Sub = ResolveSubsystem(WorldContextObject))
    {
        Sub->PauseAll();
    }
}

void UTweenBlueprintLibrary::ResumeAllTweens(UObject* WorldContextObject)
{
    if (UTweenSubsystem* Sub = ResolveSubsystem(WorldContextObject))
    {
        Sub->ResumeAll();
    }
}

void UTweenBlueprintLibrary::KillAllTweens(UObject* WorldContextObject)
{
    if (UTweenSubsystem* Sub = ResolveSubsystem(WorldContextObject))
    {
        Sub->KillAll();
    }
}

void UTweenBlueprintLibrary::PauseTweensByTag(UObject* WorldContextObject, FName Tag)
{
    if (UTweenSubsystem* Sub = ResolveSubsystem(WorldContextObject))
    {
        Sub->PauseByTag(FTweenTag(Tag));
    }
}

void UTweenBlueprintLibrary::ResumeTweensByTag(UObject* WorldContextObject, FName Tag)
{
    if (UTweenSubsystem* Sub = ResolveSubsystem(WorldContextObject))
    {
        Sub->ResumeByTag(FTweenTag(Tag));
    }
}

void UTweenBlueprintLibrary::KillTweensByTag(UObject* WorldContextObject, FName Tag)
{
    if (UTweenSubsystem* Sub = ResolveSubsystem(WorldContextObject))
    {
        Sub->KillByTag(FTweenTag(Tag));
    }
}

int32 UTweenBlueprintLibrary::GetActiveTweenCount(UObject* WorldContextObject)
{
    if (UTweenSubsystem* Sub = ResolveSubsystem(WorldContextObject))
    {
        return Sub->GetActiveTweenCount();
    }
    return 0;
}

int32 UTweenBlueprintLibrary::GetActiveSequenceCount(UObject* WorldContextObject)
{
    if (UTweenSubsystem* Sub = ResolveSubsystem(WorldContextObject))
    {
        return Sub->GetActiveSequenceCount();
    }
    return 0;
}
