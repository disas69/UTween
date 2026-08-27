#pragma once

#include "UTween.h"
#include "TweenSequence.h"
#include "TweenHandle.h"
#include "Templates/UnrealTypeTraits.h"
#include "UObject/Object.h"
#include <type_traits>
#include <utility>

/**
 * Opt-in macro layer for the UTween C++ API. All macros expand into the
 * ::UTween:: or ::UTweenDetail:: namespaces.
 */

#define UTWEEN(Obj, Start, End, Callback, Duration) \
    ::UTween::ValueMember((Obj), (Start), (End), (Duration), (Callback))

#define UTWEEN_POSITION(Obj, Start, End, Duration) \
    ::UTween::Position((Obj), (Start), (End), (Duration))

#define UTWEEN_SCALE(Obj, Start, End, Duration) \
    ::UTween::Scale((Obj), (Start), (End), (Duration))

#define UTWEEN_ROTATION(Obj, Start, End, Duration) \
    ::UTween::Rotation((Obj), (Start), (End), (Duration))

#define UTWEEN_OPACITY(Obj, Start, End, Duration) \
    ::UTween::Opacity((Obj), (Start), (End), (Duration))

#define UTWEEN_COLOR(Obj, Start, End, Duration) \
    ::UTween::Color((Obj), (Start), (End), (Duration))

namespace UTweenDetail
{
    struct FSequenceStep
    {
        enum class EKind : uint8
        {
            Tween,
            Wait,
            Callback,
            SubSequence,
        };

        EKind Kind = EKind::Tween;
        FTweenHandle Handle;
        FTweenSequenceHandle SubSequence;
        float WaitSeconds = 0.f;
        TFunction<void()> Callback;

        static FSequenceStep MakeTween(FTweenHandle H)
        {
            FSequenceStep S;
            S.Kind = EKind::Tween;
            S.Handle = MoveTemp(H);
            return S;
        }

        static FSequenceStep MakeWait(float Seconds)
        {
            FSequenceStep S;
            S.Kind = EKind::Wait;
            S.WaitSeconds = Seconds;
            return S;
        }

        static FSequenceStep MakeCallback(TFunction<void()> Fn)
        {
            FSequenceStep S;
            S.Kind = EKind::Callback;
            S.Callback = MoveTemp(Fn);
            return S;
        }

        static FSequenceStep MakeSubSequence(FTweenSequenceHandle Sub)
        {
            FSequenceStep S;
            S.Kind = EKind::SubSequence;
            S.SubSequence = MoveTemp(Sub);
            return S;
        }
    };

    inline FSequenceStep ToStep(FTweenHandle H) { return FSequenceStep::MakeTween(MoveTemp(H)); }
    inline FSequenceStep ToStep(FTweenSequenceHandle Sub) { return FSequenceStep::MakeSubSequence(MoveTemp(Sub)); }
    inline FSequenceStep ToStep(FSequenceStep S) { return S; }

    inline void ApplyStep(FTweenSequenceHandle& Seq, FSequenceStep Step, bool bParallel)
    {
        switch (Step.Kind)
        {
        case FSequenceStep::EKind::Tween:
            if (bParallel)
            {
                Seq.Join(Step.Handle);
            }
            else
            {
                Seq.Append(Step.Handle);
            }
            break;

        case FSequenceStep::EKind::Wait:
            Seq.AppendInterval(Step.WaitSeconds);
            break;

        case FSequenceStep::EKind::Callback:
            Seq.AppendCallback(MoveTemp(Step.Callback));
            break;

        case FSequenceStep::EKind::SubSequence:
            if (bParallel)
            {
                Seq.Join(Step.SubSequence);
            }
            else
            {
                Seq.Append(Step.SubSequence);
            }
            break;
        }
    }

    template <typename TObj>
    inline void BindSequenceToObject(FTweenSequenceHandle& Seq, TObj* Obj)
    {
        if constexpr (TIsDerivedFrom<TObj, UObject>::Value)
        {
            if (Obj)
            {
                Seq.SetBoundTarget(Obj);
                Seq.SetTag(FTweenTag(Obj->GetFName()));
            }
        }
    }

    template <typename TObj, typename... TArgs>
    FTweenSequenceHandle BuildSequence(TObj* Obj, TArgs&&... Args)
    {
        const UObject* WorldContext = nullptr;
        if constexpr (TIsDerivedFrom<TObj, UObject>::Value)
        {
            WorldContext = Obj;
        }
        FTweenSequenceHandle Seq = UTween::Sequence(WorldContext);
        BindSequenceToObject(Seq, Obj);

        (ApplyStep(Seq, ToStep(std::forward<TArgs>(Args)), /*bParallel=*/false), ...);
        return Seq;
    }

    template <typename TObj, typename THead, typename... TRest>
    FTweenSequenceHandle BuildParallel(TObj* Obj, THead&& Head, TRest&&... Rest)
    {
        const UObject* WorldContext = nullptr;
        if constexpr (TIsDerivedFrom<TObj, UObject>::Value)
        {
            WorldContext = Obj;
        }
        FTweenSequenceHandle Seq = UTween::Sequence(WorldContext);
        BindSequenceToObject(Seq, Obj);

        ApplyStep(Seq, ToStep(std::forward<THead>(Head)), /*bParallel=*/false);
        (ApplyStep(Seq, ToStep(std::forward<TRest>(Rest)), /*bParallel=*/true), ...);
        return Seq;
    }
}

#define UTWEEN_SEQUENCE(Obj, ...) \
    ::UTweenDetail::BuildSequence((Obj), __VA_ARGS__)

#define UTWEEN_PARALLEL(Obj, ...) \
    ::UTweenDetail::BuildParallel((Obj), __VA_ARGS__)

#define UTWEEN_WAIT(Seconds) \
    ::UTweenDetail::FSequenceStep::MakeWait((Seconds))

#define UTWEEN_DO(Lambda) \
    ::UTweenDetail::FSequenceStep::MakeCallback((Lambda))
