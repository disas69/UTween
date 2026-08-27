#include "TweenEasing.h"

// Back overshoot constants
static constexpr float C1 = 1.70158f;
static constexpr float C2 = C1 * 1.525f;
static constexpr float C3 = C1 + 1.0f;

// Elastic period constant (2*PI / 3 and 2*PI / 4.5)
static constexpr float C4 = UE_TWO_PI / 3.0f;
static constexpr float C5 = UE_TWO_PI / 4.5f;

// Bounce constants
static constexpr float N1 = 7.5625f;
static constexpr float D1 = 2.75f;

float FTweenEasing::Evaluate(ETweenEase Ease, float T)
{
    T = FMath::Clamp(T, 0.0f, 1.0f);

    switch (Ease)
    {
        case ETweenEase::Linear: return EaseLinear(T);
        case ETweenEase::InQuad: return EaseInQuad(T);
        case ETweenEase::OutQuad: return EaseOutQuad(T);
        case ETweenEase::InOutQuad: return EaseInOutQuad(T);
        case ETweenEase::InCubic: return EaseInCubic(T);
        case ETweenEase::OutCubic: return EaseOutCubic(T);
        case ETweenEase::InOutCubic: return EaseInOutCubic(T);
        case ETweenEase::InQuart: return EaseInQuart(T);
        case ETweenEase::OutQuart: return EaseOutQuart(T);
        case ETweenEase::InOutQuart: return EaseInOutQuart(T);
        case ETweenEase::InQuint: return EaseInQuint(T);
        case ETweenEase::OutQuint: return EaseOutQuint(T);
        case ETweenEase::InOutQuint: return EaseInOutQuint(T);
        case ETweenEase::InSine: return EaseInSine(T);
        case ETweenEase::OutSine: return EaseOutSine(T);
        case ETweenEase::InOutSine: return EaseInOutSine(T);
        case ETweenEase::InExpo: return EaseInExpo(T);
        case ETweenEase::OutExpo: return EaseOutExpo(T);
        case ETweenEase::InOutExpo: return EaseInOutExpo(T);
        case ETweenEase::InCirc: return EaseInCirc(T);
        case ETweenEase::OutCirc: return EaseOutCirc(T);
        case ETweenEase::InOutCirc: return EaseInOutCirc(T);
        case ETweenEase::InBack: return EaseInBack(T);
        case ETweenEase::OutBack: return EaseOutBack(T);
        case ETweenEase::InOutBack: return EaseInOutBack(T);
        case ETweenEase::InElastic: return EaseInElastic(T);
        case ETweenEase::OutElastic: return EaseOutElastic(T);
        case ETweenEase::InOutElastic: return EaseInOutElastic(T);
        case ETweenEase::InBounce: return EaseInBounce(T);
        case ETweenEase::OutBounce: return EaseOutBounce(T);
        case ETweenEase::InOutBounce: return EaseInOutBounce(T);
        default: return T;
    }
}

float FTweenEasing::EaseLinear(float T)
{
    return T;
}

float FTweenEasing::EaseInQuad(float T)
{
    return T * T;
}

float FTweenEasing::EaseOutQuad(float T)
{
    return 1.0f - (1.0f - T) * (1.0f - T);
}

float FTweenEasing::EaseInOutQuad(float T)
{
    if (T < 0.5f)
    {
        return 2.0f * T * T;
    }
    const float U = -2.0f * T + 2.0f;
    return 1.0f - (U * U) / 2.0f;
}

float FTweenEasing::EaseInCubic(float T)
{
    return T * T * T;
}

float FTweenEasing::EaseOutCubic(float T)
{
    const float U = 1.0f - T;
    return 1.0f - (U * U * U);
}

float FTweenEasing::EaseInOutCubic(float T)
{
    if (T < 0.5f)
    {
        return 4.0f * T * T * T;
    }
    const float U = -2.0f * T + 2.0f;
    return 1.0f - (U * U * U) / 2.0f;
}

float FTweenEasing::EaseInQuart(float T)
{
    const float T2 = T * T;
    return T2 * T2;
}

float FTweenEasing::EaseOutQuart(float T)
{
    const float U = 1.0f - T;
    const float U2 = U * U;
    return 1.0f - (U2 * U2);
}

float FTweenEasing::EaseInOutQuart(float T)
{
    if (T < 0.5f)
    {
        const float T2 = T * T;
        return 8.0f * T2 * T2;
    }
    const float U = -2.0f * T + 2.0f;
    const float U2 = U * U;
    return 1.0f - (U2 * U2) / 2.0f;
}

float FTweenEasing::EaseInQuint(float T)
{
    const float T2 = T * T;
    return T2 * T2 * T;
}

float FTweenEasing::EaseOutQuint(float T)
{
    const float U = 1.0f - T;
    const float U2 = U * U;
    return 1.0f - (U2 * U2 * U);
}

float FTweenEasing::EaseInOutQuint(float T)
{
    if (T < 0.5f)
    {
        const float T2 = T * T;
        return 16.0f * T2 * T2 * T;
    }
    const float U = -2.0f * T + 2.0f;
    const float U2 = U * U;
    return 1.0f - (U2 * U2 * U) / 2.0f;
}

float FTweenEasing::EaseInSine(float T)
{
    return 1.0f - FMath::Cos(T * UE_HALF_PI);
}

float FTweenEasing::EaseOutSine(float T)
{
    return FMath::Sin(T * UE_HALF_PI);
}

float FTweenEasing::EaseInOutSine(float T)
{
    return -(FMath::Cos(UE_PI * T) - 1.0f) / 2.0f;
}

float FTweenEasing::EaseInExpo(float T)
{
    return T == 0.0f ? 0.0f : FMath::Pow(2.0f, 10.0f * T - 10.0f);
}

float FTweenEasing::EaseOutExpo(float T)
{
    return T == 1.0f ? 1.0f : 1.0f - FMath::Pow(2.0f, -10.0f * T);
}

float FTweenEasing::EaseInOutExpo(float T)
{
    if (T == 0.0f)
    {
        return 0.0f;
    }
    if (T == 1.0f)
    {
        return 1.0f;
    }
    return T < 0.5f ? FMath::Pow(2.0f, 20.0f * T - 10.0f) / 2.0f : (2.0f - FMath::Pow(2.0f, -20.0f * T + 10.0f)) / 2.0f;
}

float FTweenEasing::EaseInCirc(float T)
{
    return 1.0f - FMath::Sqrt(1.0f - T * T);
}

float FTweenEasing::EaseOutCirc(float T)
{
    return FMath::Sqrt(1.0f - (T - 1.0f) * (T - 1.0f));
}

float FTweenEasing::EaseInOutCirc(float T)
{
    if (T < 0.5f)
    {
        const float U = 2.0f * T;
        return (1.0f - FMath::Sqrt(1.0f - U * U)) / 2.0f;
    }
    const float U = -2.0f * T + 2.0f;
    return (FMath::Sqrt(1.0f - U * U) + 1.0f) / 2.0f;
}

float FTweenEasing::EaseInBack(float T)
{
    return C3 * T * T * T - C1 * T * T;
}

float FTweenEasing::EaseOutBack(float T)
{
    const float U = T - 1.0f;
    const float U2 = U * U;
    return 1.0f + C3 * (U2 * U) + C1 * U2;
}

float FTweenEasing::EaseInOutBack(float T)
{
    if (T < 0.5f)
    {
        const float U = 2.0f * T;
        return (U * U * ((C2 + 1.0f) * U - C2)) / 2.0f;
    }
    const float U = 2.0f * T - 2.0f;
    return (U * U * ((C2 + 1.0f) * U + C2) + 2.0f) / 2.0f;
}

float FTweenEasing::EaseInElastic(float T)
{
    if (T == 0.0f)
    {
        return 0.0f;
    }
    if (T == 1.0f)
    {
        return 1.0f;
    }
    return -FMath::Pow(2.0f, 10.0f * T - 10.0f) * FMath::Sin((10.0f * T - 10.75f) * C4);
}

float FTweenEasing::EaseOutElastic(float T)
{
    if (T == 0.0f)
    {
        return 0.0f;
    }
    if (T == 1.0f)
    {
        return 1.0f;
    }
    return FMath::Pow(2.0f, -10.0f * T) * FMath::Sin((10.0f * T - 0.75f) * C4) + 1.0f;
}

float FTweenEasing::EaseInOutElastic(float T)
{
    if (T == 0.0f)
    {
        return 0.0f;
    }
    if (T == 1.0f)
    {
        return 1.0f;
    }
    return T < 0.5f ? -(FMath::Pow(2.0f, 20.0f * T - 10.0f) * FMath::Sin((20.0f * T - 11.125f) * C5)) / 2.0f
                    : (FMath::Pow(2.0f, -20.0f * T + 10.0f) * FMath::Sin((20.0f * T - 11.125f) * C5)) / 2.0f + 1.0f;
}

float FTweenEasing::EaseOutBounce(float T)
{
    if (T < 1.0f / D1)
    {
        return N1 * T * T;
    }
    if (T < 2.0f / D1)
    {
        T -= 1.5f / D1;
        return N1 * T * T + 0.75f;
    }
    if (T < 2.5f / D1)
    {
        T -= 2.25f / D1;
        return N1 * T * T + 0.9375f;
    }
    T -= 2.625f / D1;
    return N1 * T * T + 0.984375f;
}

float FTweenEasing::EaseInBounce(float T)
{
    return 1.0f - EaseOutBounce(1.0f - T);
}

float FTweenEasing::EaseInOutBounce(float T)
{
    return T < 0.5f ? (1.0f - EaseOutBounce(1.0f - 2.0f * T)) / 2.0f : (1.0f + EaseOutBounce(2.0f * T - 1.0f)) / 2.0f;
}
