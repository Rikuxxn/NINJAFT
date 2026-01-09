//=============================================================================
//
// イージング処理 [easing.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _EASING_H_
#define _EASING_H_

//*****************************************************************************
// イージングクラス
//*****************************************************************************
class CEasing
{
public:
    // t : 0.0f ～ 1.0f の割合
    // 戻り値 : 補間後の値 (0.0f～1.0f)

    //=========================================================================
    // 基本イージング
    //=========================================================================
    static float Linear(float t) { return t; }

    static float EaseInSine(float t) { return 1 - cosf((t * D3DX_PI) / 2); }
    static float EaseOutSine(float t) { return sinf((t * D3DX_PI) / 2); }
    static float EaseInOutSine(float t)
    {
        return -0.5f * (cosf(D3DX_PI * t) - 1);
    }

    static float EaseInQuad(float t) { return t * t; }
    static float EaseOutQuad(float t) { return t * (2 - t); }
    static float EaseInOutQuad(float t)
    {
        return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
    }

    static float EaseInCubic(float t) { return t * t * t; }
    static float EaseOutCubic(float t) { return (--t) * t * t + 1; }
    static float EaseInOutCubic(float t)
    {
        return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
    }

    static float EaseInQuart(float t) { return t * t * t * t; }
    static float EaseOutQuart(float t) { return 1 - (--t) * t * t * t; }
    static float EaseInOutQuart(float t)
    {
        return t < 0.5f ? 8 * t * t * t * t : 1 - 8 * (--t) * t * t * t;
    }

    static float EaseInQuint(float t) { return t * t * t * t * t; }
    static float EaseOutQuint(float t) { return 1 + (--t) * t * t * t * t; }
    static float EaseInOutQuint(float t)
    {
        return t < 0.5f ? 16 * t * t * t * t * t
            : 1 + 16 * (--t) * t * t * t * t;
    }

    static float EaseInExpo(float t) { return t == 0 ? 0 : powf(2, 10 * (t - 1)); }
    static float EaseOutExpo(float t) { return t == 1 ? 1 : 1 - powf(2, -10 * t); }
    static float EaseInOutExpo(float t)
    {
        if (t == 0)
        {
            return 0;
        }
        if (t == 1)
        {
            return 1;
        }

        if (t < 0.5f)
        {
            return 0.5f * powf(2, (20 * t) - 10);
        }

        return -0.5f * powf(2, (-20 * t) + 10) + 1;
    }

    static float EaseInCirc(float t) { return 1 - sqrtf(1 - t * t); }
    static float EaseOutCirc(float t) { return sqrtf(1 - (t - 1) * (t - 1)); }
    static float EaseInOutCirc(float t)
    {
        if (t < 0.5f)
        {
            return 0.5f * (1 - sqrtf(1 - 4 * t * t));
        }

        return 0.5f * (sqrtf(-((2 * t) - 3) * ((2 * t) - 1)) + 1);
    }

    static float EaseInBack(float t, float s = 1.70158f)
    {
        return t * t * ((s + 1) * t - s);
    }
    static float EaseOutBack(float t, float s = 1.70158f)
    {
        t -= 1;
        return t * t * ((s + 1) * t + s) + 1;
    }
    static float EaseInOutBack(float t, float s = 1.70158f)
    {
        s *= 1.525f;
        if (t < 0.5f) return 0.5f * (t * 2) * (t * 2) * ((s + 1) * (t * 2) - s);
        t = t * 2 - 2;
        return 0.5f * (t * t * ((s + 1) * t + s) + 2);
    }

    static float EaseInElastic(float t)
    {
        if (t == 0 || t == 1)
        {
            return t;
        }

        return -powf(2, 10 * t - 10) * sinf((t * 10 - 10.75f) * (2 * D3DX_PI / 3));
    }
    static float EaseOutElastic(float t)
    {
        if (t == 0 || t == 1)
        {
            return t;
        }

        return powf(2, -10 * t) * sinf((t * 10 - 0.75f) * (2 * D3DX_PI / 3)) + 1;
    }
    static float EaseInOutElastic(float t)
    {
        if (t == 0 || t == 1) return t;

        if (t < 0.5f)
        {
            return -0.5f * powf(2, 20 * t - 10) *
                sinf((20 * t - 11.125f) * (2 * D3DX_PI / 4.5f));
        }

        return powf(2, -20 * t + 10) *
            sinf((20 * t - 11.125f) * (2 * D3DX_PI / 4.5f)) * 0.5f + 1;
    }

    static float EaseOutBounce(float t)
    {
        if (t < (1 / 2.75f))
        {
            return 7.5625f * t * t;
        }
        else if (t < (2 / 2.75f))
        {
            return 7.5625f * (t -= 1.5f / 2.75f) * t + 0.75f;
        }
        else if (t < (2.5 / 2.75))
        {
            return 7.5625f * (t -= 2.25f / 2.75f) * t + 0.9375f;
        }
        else
        {
            return 7.5625f * (t -= 2.625f / 2.75f) * t + 0.984375f;
        }
    }

    static float EaseInBounce(float t)
    {
        return 1 - EaseOutBounce(1 - t);
    }

    static float EaseInOutBounce(float t)
    {
        return t < 0.5f
            ? (1 - EaseOutBounce(1 - 2 * t)) * 0.5f
            : (1 + EaseOutBounce(2 * t - 1)) * 0.5f;
    }

    //=========================================================================
    // 値の補間関数
    //=========================================================================
    static float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    //=========================================================================
    // イージング呼び出し関数
    //=========================================================================
    // easingType : 使いたいイージングの種類
    template <typename EaseType>
    static float Ease(float a, float b, float t, EaseType easingType)
    {
        return Lerp(a, b, easingType(t));
    }
};

#endif
