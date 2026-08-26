#pragma once

#include "CoreMinimal.h"
#include "TweenTypes.h"

struct UTWEEN_API FTweenEasing
{
    static float Evaluate(ETweenEase Ease, float T);

private:
    static float EaseLinear(float T);
    static float EaseInQuad(float T);
    static float EaseOutQuad(float T);
    static float EaseInOutQuad(float T);
    static float EaseInCubic(float T);
    static float EaseOutCubic(float T);
    static float EaseInOutCubic(float T);
    static float EaseInQuart(float T);
    static float EaseOutQuart(float T);
    static float EaseInOutQuart(float T);
    static float EaseInQuint(float T);
    static float EaseOutQuint(float T);
    static float EaseInOutQuint(float T);
    static float EaseInSine(float T);
    static float EaseOutSine(float T);
    static float EaseInOutSine(float T);
    static float EaseInExpo(float T);
    static float EaseOutExpo(float T);
    static float EaseInOutExpo(float T);
    static float EaseInCirc(float T);
    static float EaseOutCirc(float T);
    static float EaseInOutCirc(float T);
    static float EaseInBack(float T);
    static float EaseOutBack(float T);
    static float EaseInOutBack(float T);
    static float EaseInElastic(float T);
    static float EaseOutElastic(float T);
    static float EaseInOutElastic(float T);
    static float EaseInBounce(float T);
    static float EaseOutBounce(float T);
    static float EaseInOutBounce(float T);
};
