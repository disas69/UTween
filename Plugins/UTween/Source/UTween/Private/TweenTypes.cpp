#include "TweenTypes.h"

std::atomic<int64> FTweenId::Counter{0};

FTweenId FTweenId::Generate()
{
    FTweenId Id;
    Id.Value = ++Counter;
    return Id;
}
