#pragma once

#include "CoreMinimal.h"
#include "TweenTypes.h"
#include "TweenHandle.h"
#include "TweenSequence.h"
#include "TweenBlueprintTypes.generated.h"

/**
 * Blueprint-friendly FTweenHandle. Stores the int64 ID (UPROPERTY) and a
 * non-reflected weak instance pointer for O(1) resolution without subsystem lookup.
 */
USTRUCT(BlueprintType)
struct UTWEEN_API FBPTweenHandle
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "UTween")
    int64 TweenIdValue = 0;

    TWeakPtr<FTweenInstanceBase> CachedInstance;

    FBPTweenHandle() = default;
    explicit FBPTweenHandle(FTweenHandle InHandle)
        : TweenIdValue(InHandle.GetId().Value)
        , CachedInstance(InHandle.GetInstance())
    {
    }

    FTweenHandle Resolve(const UObject* WorldContext) const;
    bool IsValid(const UObject* WorldContext) const;

    FTweenId GetId() const
    {
        FTweenId Id;
        Id.Value = TweenIdValue;
        return Id;
    }
};

/** Blueprint-friendly wrapper around FTweenSequenceHandle. */
USTRUCT(BlueprintType)
struct UTWEEN_API FBPTweenSequenceHandle
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "UTween")
    int64 SequenceIdValue = 0;

    TWeakPtr<FTweenSequence> CachedSequence;

    FBPTweenSequenceHandle() = default;
    explicit FBPTweenSequenceHandle(FTweenSequenceHandle InHandle)
        : SequenceIdValue(InHandle.GetId().Value)
        , CachedSequence(InHandle.GetInstance())
    {
    }

    FTweenSequenceHandle Resolve(const UObject* WorldContext) const;
    bool IsValid(const UObject* WorldContext) const;

    FTweenId GetId() const
    {
        FTweenId Id;
        Id.Value = SequenceIdValue;
        return Id;
    }
};

/** Dynamic delegates exposed to Blueprint for generic value tween updates. */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTweenFloatUpdate, float, Value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTweenVectorUpdate, FVector, Value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTweenColorUpdate, FLinearColor, Value);
DECLARE_DYNAMIC_DELEGATE(FOnTweenDynamicCallback);
