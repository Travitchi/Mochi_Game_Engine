#pragma once
#include "defines.hpp"
#include "math_type.h"

#define M_PI 3.14159265358979323846f
#define M_PI_2	2.0f * M_PI
#define M_HALF_PI 0.5f * M_PI
#define M_QUARTER_PI 0.25f * M_PI
#define M_ONE_OVER_PI 1.0f / M_PI
#define M_ONE_OVER_2_PI 1.0f / M_PI_2
#define M_SQRT_TWO 1.41421356237309504880f
#define M_SQRT_THREE 1.73205080756887729352f
#define M_SQRT_ONE_OVER_TWO 0.70710678118654752440f
#define M_SQRT_ONE_OVER_THREE 0.57735026918962576450f
#define M_DEG2RAD_MULTIPLIER M_PI / 180.0f
#define M_RAD2DEG_MULTIPLIER 180.0f / M_PI

//multiplier to convert sec to millisec
#define M_SEC_TO_MS_MULTIPLIER 1000.0f

//multiplier to converts millisec to seconds
#define M_MS_TO_SEC_MULTIPLIER 0.001f

#define M_INFINITY 1e30f

//smallest positive number 1.0 + epsilon != 0
#define M_FLOAT_EPSILON 1.192092896e-07f

KAPI f32 msin(f32 x);
KAPI f32 mcos(f32 x);
KAPI f32 mtan(f32 x);
KAPI f32 macos(f32 x);
KAPI f32 msqrt(f32 x);
KAPI f32 mabs(f32 x);

MINLINE b8 is_power_of_2(u64 value)
{
	return (value != 0) && ((value & (value - 1)) == 0);
}