#include "stm32f10x.h"
#include "AD.h"
#include <math.h>

// 计算电阻
float Temp_get_r(void)
{
	float Vout = AD_Value[0] * 3.3f / 4095.0f;

    if (Vout <= 0.01f) // 限幅
        Vout = 0.01f;
    if (Vout >= 3.29f)
        Vout = 3.29f;

    return 10000.0f * Vout / (3.3f - Vout); //计算电阻
}


// 计算温度
float Temp_get_t(void)
{
	float R = Temp_get_r();

    if (R < 10.0f)
        R = 10.0f;

    const float B  = 3950.0f;
    const float R0 = 10000.0f;
    const float T0 = 298.15f;

    float invT = (1.0f / T0) + (1.0f / B) * log(R / R0);
    float T = 1.0f / invT - 273.15f;

    return T;
}
