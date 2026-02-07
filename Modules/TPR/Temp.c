#include "stm32f10x.h"
#include "AD.h"
#include "my_usart.h"
#include <math.h>
#define TEMP_VS      (3.3f)
#define R_FIXED_OHM  (10000.0f)
// 计算电阻
float Temp_get_r(void)
{
	float Vout = AD_Value[0] * TEMP_VS / 4095.0f;


    // 限幅，避免除0/负数
    if (Vout < 0.01f) Vout = 0.01f;
    if (Vout > (TEMP_VS - 0.01f)) Vout = (TEMP_VS - 0.01f);

    // 正确公式：Rntc = Rfixed * Vout / (Vs - Vout)
    return R_FIXED_OHM * Vout / (TEMP_VS - Vout);
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
