#include "UTweenBlueprintLibrary.h"

#include "UTween.h"
#include "UTweenSubsystem.h"
#include "UTweenModule.h"

#include "Components/Widget.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "LatentActions.h"

class FTweenLatentAction : public FPendingLatentAction
{
public:
    FName ExecutionFunction;
    int32 OutputLink;
    FWeakObjectPtr CallbackTarget;
    FTweenHandle Handle;

    FTweenLatentAction(const FLatentActionInfo& LatentInfo, FTweenHandle InHandle)
        : ExecutionFunction(LatentInfo.ExecutionFunction)
        , OutputLink(LatentInfo.Linkage)
        , CallbackTarget(LatentInfo.CallbackTarget)
        , Handle(InHandle)
    {
    }

    virtual void UpdateOperation(FLatentResponse& Response) override
    {
        const ETweenState State = Handle.GetState();
        const bool bShouldFinish = (State == ETweenState::Completed) || (State == ETweenState::Killed) || !Handle.IsValid();
        Response.FinishAndTriggerIf(bShouldFinish, ExecutionFunction, OutputLink, CallbackTarget);
    }
};

namespace
{
    UTweenSubsystem* ResolveSubsystem(const UObject* WorldContext)
    {
        return UTween::GetSubsystem(WorldContext);
    }
}

FTweenHandle FBPTweenHandle::Resolve(const UObject* WorldContext) const
{
    if (TSharedPtr<FTweenInstanceBase> Pinned = CachedInstance.Pin())
    {
        if (Pinned->Id.Value == TweenIdValue && !Pinned->IsKilled())
        {
            return FTweenHandle(Pinned);
        }
    }

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
    if (TSharedPtr<FTweenSequence> Pinned = CachedSequence.Pin())
    {
        if (Pinned->Id.Value == SequenceIdValue && !Pinned->IsKilled())
        {
            return FTweenSequenceHandle(Pinned);
        }
    }

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

FBPTweenHandle UTweenBlueprintLibrary::TweenCanvasPosition(
    UObject* WorldContextObject, UWidget* Target,
    FVector2D To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::CanvasPosition(Target, To, Duration).SetEase(Ease));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenCanvasSize(
    UObject* WorldContextObject, UWidget* Target,
    FVector2D To, float Duration, ETweenEase Ease)
{
    return FBPTweenHandle(UTween::CanvasSize(Target, To, Duration).SetEase(Ease));
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

// Handle configuration & fluent setters
FBPTweenHandle UTweenBlueprintLibrary::SetDelay(UObject* WorldContextObject, FBPTweenHandle Handle, float Delay)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.SetDelay(Delay);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::SetLoops(UObject* WorldContextObject, FBPTweenHandle Handle, int32 Count, ETweenLoopMode Mode)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.SetLoops(Count, Mode);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::SetEase(UObject* WorldContextObject, FBPTweenHandle Handle, ETweenEase Ease)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.SetEase(Ease);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::SetRelative(UObject* WorldContextObject, FBPTweenHandle Handle, bool bRelative)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.SetRelative(bRelative);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::SetTimeMode(UObject* WorldContextObject, FBPTweenHandle Handle, ETweenTimeMode Mode)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.SetTimeMode(Mode);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::SetTag(UObject* WorldContextObject, FBPTweenHandle Handle, FName Tag)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.SetTag(FTweenTag(Tag));
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::FromFloat(UObject* WorldContextObject, FBPTweenHandle Handle, float FromValue)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.From(FromValue);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::FromVector2D(UObject* WorldContextObject, FBPTweenHandle Handle, FVector2D FromValue)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.From(FromValue);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::FromVector(UObject* WorldContextObject, FBPTweenHandle Handle, FVector FromValue)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.From(FromValue);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::FromRotator(UObject* WorldContextObject, FBPTweenHandle Handle, FRotator FromValue)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.From(FromValue);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::FromColor(UObject* WorldContextObject, FBPTweenHandle Handle, FLinearColor FromValue)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.From(FromValue);
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::OnTweenComplete(UObject* WorldContextObject, FBPTweenHandle Handle, const FOnTweenDynamicCallback& Callback)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.OnComplete([Callback]()
    {
        Callback.ExecuteIfBound();
    });
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::OnTweenStart(UObject* WorldContextObject, FBPTweenHandle Handle, const FOnTweenDynamicCallback& Callback)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.OnStart([Callback]()
    {
        Callback.ExecuteIfBound();
    });
    return Handle;
}

FBPTweenHandle UTweenBlueprintLibrary::OnTweenKill(UObject* WorldContextObject, FBPTweenHandle Handle, const FOnTweenDynamicCallback& Callback)
{
    FTweenHandle Live = Handle.Resolve(WorldContextObject);
    Live.OnKill([Callback]()
    {
        Callback.ExecuteIfBound();
    });
    return Handle;
}

void UTweenBlueprintLibrary::PlayTween(UObject* WorldContextObject, FBPTweenHandle Handle)
{
    Handle.Resolve(WorldContextObject).Play();
}

void UTweenBlueprintLibrary::RestartTween(UObject* WorldContextObject, FBPTweenHandle Handle)
{
    Handle.Resolve(WorldContextObject).Restart();
}

void UTweenBlueprintLibrary::CompleteTween(UObject* WorldContextObject, FBPTweenHandle Handle)
{
    Handle.Resolve(WorldContextObject).Complete();
}

// Sequence configuration
FBPTweenSequenceHandle UTweenBlueprintLibrary::SetSequenceLoops(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, int32 Count, ETweenLoopMode Mode)
{
    FTweenSequenceHandle Live = Sequence.Resolve(WorldContextObject);
    Live.SetLoops(Count, Mode);
    return Sequence;
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::SetSequenceTimeMode(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, ETweenTimeMode Mode)
{
    FTweenSequenceHandle Live = Sequence.Resolve(WorldContextObject);
    Live.SetTimeMode(Mode);
    return Sequence;
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::SetSequenceTag(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, FName Tag)
{
    FTweenSequenceHandle Live = Sequence.Resolve(WorldContextObject);
    Live.SetTag(FTweenTag(Tag));
    return Sequence;
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::OnSequenceComplete(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, const FOnTweenDynamicCallback& Callback)
{
    FTweenSequenceHandle Live = Sequence.Resolve(WorldContextObject);
    Live.OnComplete([Callback]()
    {
        Callback.ExecuteIfBound();
    });
    return Sequence;
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::OnSequenceKill(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, const FOnTweenDynamicCallback& Callback)
{
    FTweenSequenceHandle Live = Sequence.Resolve(WorldContextObject);
    Live.OnKill([Callback]()
    {
        Callback.ExecuteIfBound();
    });
    return Sequence;
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::AppendSubSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, FBPTweenSequenceHandle SubSequence)
{
    FTweenSequenceHandle Parent = Sequence.Resolve(WorldContextObject);
    FTweenSequenceHandle Child = SubSequence.Resolve(WorldContextObject);
    Parent.Append(Child);
    return Sequence;
}

FBPTweenSequenceHandle UTweenBlueprintLibrary::JoinSubSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence, FBPTweenSequenceHandle SubSequence)
{
    FTweenSequenceHandle Parent = Sequence.Resolve(WorldContextObject);
    FTweenSequenceHandle Child = SubSequence.Resolve(WorldContextObject);
    Parent.Join(Child);
    return Sequence;
}

void UTweenBlueprintLibrary::RestartSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence)
{
    Sequence.Resolve(WorldContextObject).Restart();
}

void UTweenBlueprintLibrary::ResumeSequence(UObject* WorldContextObject, FBPTweenSequenceHandle Sequence)
{
    Sequence.Resolve(WorldContextObject).Resume();
}

// Latent Actions
void UTweenBlueprintLibrary::TweenActorLocationLatent(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    AActor* Target,
    FVector To,
    float Duration,
    ETweenEase Ease)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        return;
    }

    FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
    if (LatentActionManager.FindExistingAction<FTweenLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
    {
        FTweenHandle Handle = UTween::ActorLocation(Target, To, Duration).SetEase(Ease);
        LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FTweenLatentAction(LatentInfo, Handle));
    }
}

void UTweenBlueprintLibrary::TweenActorRotationLatent(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    AActor* Target,
    FRotator To,
    float Duration,
    ETweenEase Ease)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        return;
    }

    FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
    if (LatentActionManager.FindExistingAction<FTweenLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
    {
        FTweenHandle Handle = UTween::ActorRotation(Target, To, Duration).SetEase(Ease);
        LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FTweenLatentAction(LatentInfo, Handle));
    }
}

void UTweenBlueprintLibrary::TweenActorScaleLatent(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    AActor* Target,
    FVector To,
    float Duration,
    ETweenEase Ease)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        return;
    }

    FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
    if (LatentActionManager.FindExistingAction<FTweenLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
    {
        FTweenHandle Handle = UTween::ActorScale(Target, To, Duration).SetEase(Ease);
        LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FTweenLatentAction(LatentInfo, Handle));
    }
}

void UTweenBlueprintLibrary::TweenWidgetPositionLatent(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    UWidget* Target,
    FVector2D To,
    float Duration,
    ETweenEase Ease)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        return;
    }

    FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
    if (LatentActionManager.FindExistingAction<FTweenLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
    {
        FTweenHandle Handle = UTween::WidgetPosition(Target, To, Duration).SetEase(Ease);
        LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FTweenLatentAction(LatentInfo, Handle));
    }
}

// Juice (Punch & Shake)
FBPTweenHandle UTweenBlueprintLibrary::TweenActorPunchPosition(
    UObject* WorldContextObject,
    AActor* Target,
    FVector Punch,
    float Duration,
    int32 Vibrato,
    float Elasticity)
{
    return FBPTweenHandle(UTween::PunchPosition(Target, Punch, Duration, Vibrato, Elasticity));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenActorPunchScale(
    UObject* WorldContextObject,
    AActor* Target,
    FVector Punch,
    float Duration,
    int32 Vibrato,
    float Elasticity)
{
    return FBPTweenHandle(UTween::PunchScale(Target, Punch, Duration, Vibrato, Elasticity));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenActorShakePosition(
    UObject* WorldContextObject,
    AActor* Target,
    float Strength,
    float Duration,
    int32 Vibrato,
    float Randomness)
{
    return FBPTweenHandle(UTween::ShakePosition(Target, Strength, Duration, Vibrato, Randomness));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenWidgetPunchPosition(
    UObject* WorldContextObject,
    UWidget* Target,
    FVector2D Punch,
    float Duration,
    int32 Vibrato,
    float Elasticity)
{
    return FBPTweenHandle(UTween::PunchPosition(Target, Punch, Duration, Vibrato, Elasticity));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenWidgetPunchScale(
    UObject* WorldContextObject,
    UWidget* Target,
    FVector2D Punch,
    float Duration,
    int32 Vibrato,
    float Elasticity)
{
    return FBPTweenHandle(UTween::PunchScale(Target, Punch, Duration, Vibrato, Elasticity));
}

FBPTweenHandle UTweenBlueprintLibrary::TweenWidgetShakePosition(
    UObject* WorldContextObject,
    UWidget* Target,
    float Strength,
    float Duration,
    int32 Vibrato)
{
    return FBPTweenHandle(UTween::ShakePosition(Target, Strength, Duration, Vibrato));
}
