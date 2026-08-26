#pragma once

#include "CoreMinimal.h"
#include <atomic>
#include "TweenTypes.generated.h"

UENUM(BlueprintType)
enum class ETweenEase : uint8
{
    Linear,
    InQuad,
    OutQuad,
    InOutQuad,
    InCubic,
    OutCubic,
    InOutCubic,
    InQuart,
    OutQuart,
    InOutQuart,
    InQuint,
    OutQuint,
    InOutQuint,
    InSine,
    OutSine,
    InOutSine,
    InExpo,
    OutExpo,
    InOutExpo,
    InCirc,
    OutCirc,
    InOutCirc,
    InBack,
    OutBack,
    InOutBack,
    InElastic,
    OutElastic,
    InOutElastic,
    InBounce,
    OutBounce,
    InOutBounce,
};

UENUM(BlueprintType)
enum class ETweenLoopMode : uint8
{
    None,
    Restart,
    Yoyo,
};

UENUM(BlueprintType)
enum class ETweenTimeMode : uint8
{
    GameTime,
    RealTime,
};

UENUM(BlueprintType)
enum class ETweenState : uint8
{
    Pending,
    Playing,
    Paused,
    Completed,
    Killed,
};

USTRUCT(BlueprintType)
struct UTWEEN_API FTweenId
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int64 Value = 0;

    bool IsValid() const { return Value != 0; }

    bool operator==(const FTweenId& Other) const { return Value == Other.Value; }
    bool operator!=(const FTweenId& Other) const { return Value != Other.Value; }

    friend uint32 GetTypeHash(const FTweenId& Id) { return GetTypeHash(Id.Value); }

    static FTweenId Generate();

private:
    static std::atomic<int64> Counter;
};

USTRUCT(BlueprintType)
struct UTWEEN_API FTweenTag
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Name;

    FTweenTag() = default;
    explicit FTweenTag(FName InName) : Name(InName) {}

    bool IsValid() const { return !Name.IsNone(); }

    bool operator==(const FTweenTag& Other) const { return Name == Other.Name; }
    bool operator!=(const FTweenTag& Other) const { return Name != Other.Name; }

    friend uint32 GetTypeHash(const FTweenTag& Tag) { return GetTypeHash(Tag.Name); }
};
