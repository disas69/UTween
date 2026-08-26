#include "TweenHandle.h"
#include "TweenInstance.h"
#include "UTweenModule.h"

FTweenHandle::FTweenHandle(TSharedPtr<FTweenInstanceBase> InInstance)
    : InstancePtr(InInstance)
    , Id(InInstance.IsValid() ? InInstance->Id : FTweenId{})
{
}

FTweenInstanceBase* FTweenHandle::Pin() const
{
    TSharedPtr<FTweenInstanceBase> Pinned = InstancePtr.Pin();
    return Pinned.Get();
}

FTweenHandle& FTweenHandle::SetEase(ETweenEase InEase)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->Ease = InEase;
    }
    return *this;
}

FTweenHandle& FTweenHandle::SetDelay(float InDelay)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->Delay = FMath::Max(0.f, InDelay);
    }
    return *this;
}

FTweenHandle& FTweenHandle::SetLoops(int32 InLoopCount, ETweenLoopMode InMode)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->LoopCount = InLoopCount;
        Inst->LoopMode = (InLoopCount == 0) ? ETweenLoopMode::None : InMode;
    }
    return *this;
}

FTweenHandle& FTweenHandle::SetAutoKill(bool bInAutoKill)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->bAutoKill = bInAutoKill;
    }
    return *this;
}

FTweenHandle& FTweenHandle::SetTimeMode(ETweenTimeMode InMode)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->TimeMode = InMode;
    }
    return *this;
}

FTweenHandle& FTweenHandle::SetTag(FTweenTag InTag)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->Tag = InTag;
    }
    return *this;
}

FTweenHandle& FTweenHandle::SetRelative(bool bInRelative)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->bIsRelative = bInRelative;
    }
    return *this;
}

namespace
{
    template <typename T>
    void ApplyFromTyped(FTweenInstanceBase* Inst, const T& InValue)
    {
        if (!Inst)
        {
            return;
        }
        if (!Inst->IsType<T>())
        {
            UE_LOG(LogUTween, Warning,
                TEXT("FTweenHandle::From type mismatch (tween Id=%lld); ignoring."),
                Inst->Id.Value);
            return;
        }
        auto* Typed = static_cast<TTweenInstance<T>*>(Inst);
        Typed->StartValue = InValue;
        Typed->bUseFrom = true;
    }
}

FTweenHandle& FTweenHandle::From(float InValue)
{
    ApplyFromTyped<float>(Pin(), InValue);
    return *this;
}

FTweenHandle& FTweenHandle::From(const FVector2D& InValue)
{
    ApplyFromTyped<FVector2D>(Pin(), InValue);
    return *this;
}

FTweenHandle& FTweenHandle::From(const FVector& InValue)
{
    ApplyFromTyped<FVector>(Pin(), InValue);
    return *this;
}

FTweenHandle& FTweenHandle::From(const FRotator& InValue)
{
    ApplyFromTyped<FRotator>(Pin(), InValue);
    return *this;
}

FTweenHandle& FTweenHandle::From(const FLinearColor& InValue)
{
    ApplyFromTyped<FLinearColor>(Pin(), InValue);
    return *this;
}

FTweenHandle& FTweenHandle::OnStart(TFunction<void()> Callback)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->OnStartCallback = MoveTemp(Callback);
    }
    return *this;
}

FTweenHandle& FTweenHandle::OnUpdate(TFunction<void()> Callback)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->OnUpdateCallback = MoveTemp(Callback);
    }
    return *this;
}

FTweenHandle& FTweenHandle::OnComplete(TFunction<void()> Callback)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->OnCompleteCallback = MoveTemp(Callback);
    }
    return *this;
}

FTweenHandle& FTweenHandle::OnKill(TFunction<void()> Callback)
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->OnKillCallback = MoveTemp(Callback);
    }
    return *this;
}

void FTweenHandle::Play()
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->Play();
    }
}

void FTweenHandle::Restart()
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->Restart();
    }
}

void FTweenHandle::Pause()
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->Pause();
    }
}

void FTweenHandle::Resume()
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->Resume();
    }
}

void FTweenHandle::Complete()
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->Complete();
    }
}

void FTweenHandle::Kill()
{
    if (FTweenInstanceBase* Inst = Pin())
    {
        Inst->Kill();
    }
}

bool FTweenHandle::IsValid() const
{
    const FTweenInstanceBase* Inst = Pin();
    return Inst != nullptr && !Inst->IsKilled();
}

bool FTweenHandle::IsPlaying() const
{
    const FTweenInstanceBase* Inst = Pin();
    return Inst != nullptr && Inst->State == ETweenState::Playing;
}

ETweenState FTweenHandle::GetState() const
{
    if (const FTweenInstanceBase* Inst = Pin())
    {
        return Inst->State;
    }
    return ETweenState::Killed;
}
