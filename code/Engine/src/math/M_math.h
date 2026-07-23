#pragma once
#include "defines.hpp"
#include "math_type.h"
#include "M_memory.h"

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

KAPI i32 mrandom();
KAPI i32 mrandom_in_range(i32 min, i32 max);
KAPI f32 fmrandom();
KAPI f32 fmrandom_in_range(f32 min, f32 max);



//Vector 2 Section

//creates a vector with given info
MINLINE vect2 vect2_create(f32 x, f32 y)
{
	vect2 o_vector;
	o_vector.x = x;
	o_vector.y = y;
	return o_vector;
}


//shortcuts for setting x and y to 0 or 1
MINLINE vect2 vect2_zero()
{
	return vect2 {0.0f , 0.0f};
}

MINLINE vect2 vect2_one()
{
	return vect2 { 1.0f, 1.0f };
}

//directional vectors

MINLINE vect2 vect2_up()
{
	return vect2{ 0.0f, 1.0f };
}

MINLINE vect2 vect2_down()
{
	return vect2{ 0.0f, -1.0f };
}

MINLINE vect2 vect2_left()
{
	return vect2{ -1.0f, 0.0f };
}

MINLINE vect2 vect2_right()
{
	return vect2{ 1.0f, 0.0f };
}

//math between vectors
MINLINE vect2 vect2_add(vect2 vector_1, vect2 vector_2)
{
	return vect2{ vector_1.x + vector_2.x, vector_1.y + vector_2.y };
}

MINLINE vect2 vect2_sub(vect2 vector_1, vect2 vector_2)
{
	return vect2{ vector_1.x - vector_2.x, vector_1.y - vector_2.y };
}

MINLINE vect2 vect2_mult(vect2 vector_1, vect2 vector_2)
{
	return vect2{ vector_1.x * vector_2.x, vector_1.y * vector_2.y };
}

MINLINE vect2 vect2_div(vect2 vector_1, vect2 vector_2)
{
	return vect2{ vector_1.x / vector_2.x, vector_1.y / vector_2.y };
}

//returns the squared length of the vector
MINLINE f32 vect2_length_squared(vect2 vector)
{
	return vector.x * vector.x + vector.y * vector.y;
}


MINLINE f32 vect2_length(vect2 vector)
{
	return msqrt(vect2_length_squared(vector));
}

//normalizes into a unit vector
MINLINE void vect2_normalize(vect2* vector)
{
	const f32 length = vect2_length(*vector);
	vector->x /= length;
	vector->y /= length;
}


//returns normalized copy to the supplied vector
MINLINE vect2 vect2_normalized(vect2 vector)
{
	vect2_normalize(&vector);
	return vector;

}

//compares 2 vectors to see if it's less of the tollerance 
MINLINE b8 vect2_compare(vect2 vector_1, vect2 vector_2, f32 tollerance)
{
	if (mabs(vector_1.x - vector_2.x) > tollerance)
	{
		return FALSE;
	}
	if (mabs(vector_1.y - vector_2.y) > tollerance)
	{
		return FALSE;
	}

	return TRUE;
}

//return the distance between 2 vectors
MINLINE f32 vect2_dist(vect2 vector_1, vect2 vector_2)
{
	vect2 d = vect2{ vector_1.x - vector_2.x, vector_1.y - vector_2.y };
	return vect2_length(d);
}



//Vector 3 Section

//creates a 3d vector
MINLINE	vect3 vect3_create(f32 x, f32 y, f32 z)
{
	return vect3{ x, y, z };
}

MINLINE vect3 vect3_from_vect4(vect4 vector)
{
	return vect3{ vector.x, vector.y, vector.z };
}

MINLINE vect4 vect3_to_vect4(vect3 vector, f32 w)
{
	return vect4{ vector.x, vector.y, vector.z, w};
}

MINLINE vect3 vect3_zero()
{
	return vect3{ 0.0f, 0.0f, 0.0f};
}

MINLINE vect3 vect3_one()
{
	return vect3{ 1.0f, 1.0f, 1.0f };
}

MINLINE vect3 vect3_up() {
	return vect3{ 0.0f, 1.0f, 0.0f };
}

MINLINE vect3 vect3_down() {
	return vect3{ 0.0f, -1.0f, 0.0f };
}

MINLINE vect3 vect3_right() {
	return vect3{ 1.0f, 0.0f, 0.0f };
}

MINLINE vect3 vect3_left() {
	return vect3{ -1.0f, 0.0f, 0.0f };
}


// In OpenGL forward goes into negative Z!
MINLINE vect3 vec3_forward() {
	return vect3{ 0.0f, 0.0f, -1.0f }; 
}
// In OpenGL backward goes into positive Z!
MINLINE vect3 vect3_backward() {
	return vect3{ 0.0f, 0.0f, 1.0f };
}

MINLINE vect3 vect3_add(vect3 vector_1, vect3 vector_2)
{
	return vect3{ vector_1.x + vector_2.x, vector_1.y + vector_2.y, vector_1.z + vector_2.z };
}

MINLINE vect3 vect3_sub(vect3 vector_1, vect3 vector_2)
{
	return vect3{ vector_1.x - vector_2.x, vector_1.y - vector_2.y, vector_1.z - vector_2.z };
}

MINLINE vect3 vect3_mult(vect3 vector_1, vect3 vector_2)
{
	return vect3{ vector_1.x * vector_2.x, vector_1.y * vector_2.y, vector_1.z * vector_2.z };
}

MINLINE vect3 vect3_mult_scale(vect3 vector_1, f32 scalar)
{
	return vect3{ vector_1.x * scalar, vector_1.y * scalar, vector_1.z * scalar };
}

MINLINE vect3 vect3_div(vect3 vector_1, vect3 vector_2)
{
	return vect3{ vector_1.x / vector_2.x, vector_1.y / vector_2.y, vector_1.z / vector_2.z };
}

MINLINE f32 vect3_length_squared(vect3 vector)
{
	return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
}

MINLINE f32 vect3_length(vect3 vector)
{
	return msqrt(vect3_length_squared(vector));
}

MINLINE void vect3_normalize(vect3* vector)
{
	const f32 length = vect3_length(*vector);
	vector->x /= length;
	vector->y /= length;
	vector->z /= length;
}

MINLINE vect3 vect3_normalized(vect3 vector)
{
	vect3_normalize(&vector);
	return vector;

}


//typically used to calculate teh difference in direction, and it's called the dot product
MINLINE f32 vect3_dot(vect3 vector_1, vect3 vector_2)
{
	f32 p = 0;
	p += vector_1.x * vector_2.x;
	p += vector_1.y * vector_2.y;
	p += vector_1.z * vector_2.z;
	return p;
}

//cross  product returns a new vector which is orthoganal to both provided vectors
MINLINE vect3 vect3_cross(vect3 vector_1, vect3 vector_2)
{
	return vect3
	{
		vector_1.y * vector_2.z - vector_1.z * vector_2.y,
		vector_1.z* vector_2.x - vector_1.x * vector_2.z,
		vector_1.x* vector_2.y - vector_1.y * vector_2.x
	};
}

MINLINE b8 vect3_compare(vect3 vector_1, vect3 vector_2, f32 tollerance)
{
	if (mabs(vector_1.x - vector_2.x) > tollerance)
	{
		return FALSE;
	}
	if (mabs(vector_1.y - vector_2.y) > tollerance)
	{
		return FALSE;
	}
	if (mabs(vector_1.z - vector_2.z) > tollerance)
	{
		return FALSE;
	}

	return TRUE;
}

MINLINE f32 vect3_dist(vect3 vector_1, vect3 vector_2)
{
	vect3 d = vect3{ vector_1.x - vector_2.x, vector_1.y - vector_2.y, vector_1.z - vector_2.z };
	return vect3_length(d);
}



//Vector 4 Section

MINLINE	vect4 vect4_create(f32 x, f32 y, f32 z, f32 w)
{
	vect4 o_vect;
#if defined(KUSE_SIMD)
	o_vect.data = _mm_setr_ps(x, y, z, w);
#else
	o_vect.x = x;
	o_vect.y = y;
	o_vect.z = z;
	o_vect.w = w;
#endif
	return o_vect;
}

MINLINE vect3 vect4_to_vect3(vect4 vector)
{
	return vect3{ vector.x, vector.y, vector.z };
}

MINLINE vect4 vect4_from_vect3(vect3 vector, f32 w)
{
#if defined(KUSE_SIMD)
	vect4 o_vect;
	o_vect.data = mm_setr_ps(x, y, z, w);
	return o_vect;
#else
	return vect4{ vector.x, vector.y, vector.z, w };
#endif
}

MINLINE vect4 vect4_zero()
{
	return vect4{ 0.0f, 0.0f, 0.0f, 0.0f };
}

MINLINE vect4 vect4_one()
{
	return vect4{ 1.0f, 1.0f, 1.0f, 1.0f };
}

MINLINE vect4 vect4_add(vect4 vector_1, vect4 vector_2)
{
	vect4 result;
	for (u64 i = 0; i < 4; i++)
	{
		result.elements[i] = vector_1.elements[i] + vector_2.elements[i];
	}
	return result;
}

MINLINE vect4 vect4_sub(vect4 vector_1, vect4 vector_2)
{
	vect4 result;
	for (u64 i = 0; i < 4; i++)
	{
		result.elements[i] = vector_1.elements[i] - vector_2.elements[i];
	}
	return result;
}

MINLINE vect4 vect4_mult(vect4 vector_1, vect4 vector_2)
{
	vect4 result;
	for (u64 i = 0; i < 4; i++)
	{
		result.elements[i] = vector_1.elements[i] * vector_2.elements[i];
	}
	return result;
}

MINLINE vect4 vect4_div(vect4 vector_1, vect4 vector_2)
{
	vect4 result;
	for (u64 i = 0; i < 4; i++)
	{
		result.elements[i] = vector_1.elements[i] / vector_2.elements[i];
	}
	return result;
}

MINLINE f32 vect4_length_squared(vect4 vector)
{
	return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w;
}

MINLINE f32 vect4_length(vect4 vector)
{
	return msqrt(vect4_length_squared(vector));
}

MINLINE void vect4_normalize(vect4* vector)
{
	const f32 length = vect4_length(*vector);
	vector->x /= length;
	vector->y /= length;
	vector->z /= length;
	vector->w /= length;
}

MINLINE vect4 vect4_normalized(vect4 vector)
{
	vect4_normalize(&vector);
	return vector;

}

MINLINE f32 vect4_dot(f32 a0, f32 a1, f32 a2, f32 a3, f32 b0, f32 b1, f32 b2, f32 b3)
{
	f32 p;
	p = a0 * b0 + a1 * b1 + a2 * b2 + a3 * b3;
	return p;
}



//Matrix Section

//identity matrix is a default matrix
MINLINE mat4 mat4_id()
{
	mat4 o_matrix;
	Mzero_memory(o_matrix.data, sizeof(f32) * 16);
	o_matrix.data[0] = 1.0f;
	o_matrix.data[5] = 1.0f;
	o_matrix.data[10] = 1.0f;
	o_matrix.data[15] = 1.0f;
	return o_matrix;
}

MINLINE mat4 mat4_mult(mat4 matrix1, mat4 matrix2)
{
	mat4 o_matrix = mat4_id();
	const f32* m1_ptr = matrix1.data;
	const f32* m2_ptr = matrix2.data;
	f32* dst_ptr = o_matrix.data;

	for (i32 i = 0; i < 4; ++i)
	{
		for (i32 j = 0; j < 4; ++j)
		{
			*dst_ptr = m1_ptr[0] * m2_ptr[0 + j] + m1_ptr[1] * m2_ptr[4 + j] + m1_ptr[2] * m2_ptr[8 + j] + m1_ptr[3] * m2_ptr[12 + j];
			dst_ptr++;
		}
		m1_ptr += 4;
	}
	return o_matrix;
}

//orthographic projection matrix
MINLINE mat4 mat4_orthographic(f32 left, f32 right, f32 top, f32 bottom, f32 near_clip, f32 far_clip)
{
	mat4 o_matrix = mat4_id();

	//diagonal scaling
	o_matrix.data[0] = 2.0f / (right - left);
	o_matrix.data[5] = 2.0f / (top - bottom);
	o_matrix.data[10] = -2.0f / (far_clip - near_clip);

	//translation cloumn
	o_matrix.data[12] = -(right + left) / (right - left);
	o_matrix.data[13] = -(top + bottom) / (top - bottom);
	o_matrix.data[14] = -(far_clip + near_clip) / (far_clip - near_clip);

	return o_matrix;
}

//perspective matrix
MINLINE mat4 mat4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near_clip, f32 far_clip)
{
	f32 half_tan_fov = mtan(fov_radians * 0.5f);
	mat4 o_matrix;
	Mzero_memory(o_matrix.data, sizeof(f32) * 16);
	o_matrix.data[0] = 1.0f / (aspect_ratio * half_tan_fov);
	o_matrix.data[5] = 1.0f / half_tan_fov;
	o_matrix.data[10] = -((far_clip + near_clip) / (far_clip - near_clip));
	o_matrix.data[11] = -1.0f;
	o_matrix.data[14] = -((2.0f * far_clip * near_clip) / (far_clip - near_clip));
	return o_matrix;
}


MINLINE mat4 mat4_look_at(vect3 position, vect3 target, vect3 up)
{
	mat4 o_matrix;
	vect3 z_axis;

	z_axis.x = target.x - position.x;
	z_axis.y = target.y - position.y;
	z_axis.z = target.z - position.z;

	z_axis = vect3_normalized(z_axis);
	vect3 x_axis = vect3_normalized(vect3_cross(z_axis, up));
	vect3 y_axis = vect3_cross(x_axis, z_axis);

	o_matrix.data[0] = x_axis.x;
	o_matrix.data[1] = y_axis.x;
	o_matrix.data[2] = -z_axis.x;
	o_matrix.data[3] = 0;
	o_matrix.data[4] = x_axis.y;
	o_matrix.data[5] = y_axis.y;
	o_matrix.data[6] = -z_axis.y;
	o_matrix.data[7] = 0;
	o_matrix.data[8] = x_axis.z;
	o_matrix.data[9] = y_axis.z;
	o_matrix.data[10] = -z_axis.z;
	o_matrix.data[11] = 0;
	o_matrix.data[12] = -vect3_dot(x_axis, position);
	o_matrix.data[13] = -vect3_dot(y_axis, position);
	o_matrix.data[14] = vect3_dot(z_axis, position);
	o_matrix.data[15] = 1.0f;

	return o_matrix;

}


//Transposed, flipping the matrix diagonally so that all the rows become columns, and all the columns become rows. useful for dynamic 3d lighting on a object

MINLINE mat4 mat4_transposed(mat4 matrix) {
	mat4 o_matrix = mat4_id();

	o_matrix.data[0] = matrix.data[0];
	o_matrix.data[1] = matrix.data[4];
	o_matrix.data[2] = matrix.data[8];
	o_matrix.data[3] = matrix.data[12];

	o_matrix.data[4] = matrix.data[1];
	o_matrix.data[5] = matrix.data[5];
	o_matrix.data[6] = matrix.data[9];
	o_matrix.data[7] = matrix.data[13];

	o_matrix.data[8] = matrix.data[2];
	o_matrix.data[9] = matrix.data[6];
	o_matrix.data[10] = matrix.data[10];
	o_matrix.data[11] = matrix.data[14];

	o_matrix.data[12] = matrix.data[3];
	o_matrix.data[13] = matrix.data[7];
	o_matrix.data[14] = matrix.data[11];
	o_matrix.data[15] = matrix.data[15];

	return o_matrix;
}


//inverts a matrix, used for an example tprojection matrix that took something from 3d space to 2d space, and now we want to go back to 3d space, we need to invert the matrix
MINLINE mat4 mat4_inverse(mat4 matrix)
{
	const f32* m = matrix.data;

	f32 t0 = m[10] * m[15];
	f32 t1 = m[14] * m[11];
	f32 t2 = m[6] * m[15];
	f32 t3 = m[14] * m[7];
	f32 t4 = m[6] * m[11];
	f32 t5 = m[10] * m[7];
	f32 t6 = m[2] * m[15];
	f32 t7 = m[14] * m[3];
	f32 t8 = m[2] * m[11];
	f32 t9 = m[10] * m[3];
	f32 t10 = m[2] * m[7];
	f32 t11 = m[6] * m[3];
	f32 t12 = m[8] * m[13];
	f32 t13 = m[12] * m[9];
	f32 t14 = m[4] * m[13];
	f32 t15 = m[12] * m[5];
	f32 t16 = m[4] * m[9];
	f32 t17 = m[8] * m[5];
	f32 t18 = m[0] * m[13];
	f32 t19 = m[12] * m[1];
	f32 t20 = m[0] * m[9];
	f32 t21 = m[8] * m[1];
	f32 t22 = m[0] * m[5];
	f32 t23 = m[4] * m[1];

	mat4 o_matrix;
	f32* o = o_matrix.data;

	o[0] = (t0 * m[5] + t3 * m[9] + t4 * m[13]) - (t1 * m[5] - t2 * m[9] - t5 * m[13]);
	o[1] = (t1 * m[1] + t6 * m[9] + t9 * m[13]) - (t0 * m[1] + t7 * m[9] + t8 * m[13]);
	o[2] = (t2 * m[1] + t7 * m[5] + t10 * m[13]) - (t3 * m[1] + t6 * m[5] + t11 * m[13]);
	o[3] = (t5 * m[1] + t8 * m[5] + t11 * m[9]) - (t4 * m[1] + t9 * m[5] + t10 * m[9]);

	f32 d = 1.0f / (m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3]);

	o[0] = d * o[0];
	o[1] = d * o[1];
	o[2] = d * o[2];
	o[3] = d * o[3];
	o[4] = d * ((t1 * m[4] + t2 * m[8] + t5 * m[12]) - (t0 * m[4] + t3 * m[8] + t4 * m[12]));
	o[5] = d * ((t0 * m[0] + t7 * m[8] + t8 * m[12]) - (t1 * m[0] + t6 * m[8] + t9 * m[12]));
	o[6] = d * ((t3 * m[0] + t6 * m[4] + t11 * m[12]) - (t2 * m[0] + t7 * m[4] + t10 * m[12]));
	o[7] = d * ((t4 * m[0] + t9 * m[4] + t10 * m[8]) - (t5 * m[0] + t8 * m[4] + t11 * m[8]));
	o[8] = d * ((t12 * m[7] + t15 * m[11] + t16 * m[15]) - (t13 * m[7] + t14 * m[11] + t17 * m[15]));
	o[9] = d * ((t13 * m[3] + t18 * m[11] + t21 * m[15]) - (t12 * m[3] + t19 * m[11] + t20 * m[15]));
	o[10] = d * ((t14 * m[3] + t19 * m[7] + t22 * m[15]) - (t15 * m[3] + t18 * m[7] + t23 * m[15]));
	o[11] = d * ((t17 * m[3] + t20 * m[7] + t23 * m[11]) - (t16 * m[3] + t21 * m[7] + t22 * m[11]));
	o[12] = d * ((t14 * m[10] + t17 * m[14] + t13 * m[6]) - (t16 * m[14] + t12 * m[6] + t15 * m[10]));
	o[13] = d * ((t20 * m[14] + t12 * m[2] + t19 * m[10]) - (t18 * m[10] + t21 * m[14] + t13 * m[2]));
	o[14] = d * ((t18 * m[6] + t23 * m[14] + t15 * m[2]) - (t22 * m[14] + t14 * m[2] + t19 * m[6]));
	o[15] = d * ((t22 * m[10] + t16 * m[2] + t21 * m[6]) - (t20 * m[6] + t23 * m[10] + t17 * m[2]));

	return o_matrix;
}

//translation matrix, hold positional information, used to move objects in 3d space
MINLINE mat4 mat4_translation(vect3 position)
{
	mat4 o_matrix = mat4_id();
	o_matrix.data[12] = position.x;
	o_matrix.data[13] = position.y;
	o_matrix.data[14] = position.z;
	return o_matrix;
}


//scaling matrix, used to scale objects in 3d space
MINLINE mat4 mat4_scale(vect3 scale)
{
	mat4 o_matrix = mat4_id();
	o_matrix.data[0] = scale.x;
	o_matrix.data[5] = scale.y;
	o_matrix.data[10] = scale.z;
	return o_matrix;
}


MINLINE mat4 mat4_eulero_x(f32 angle_radians)
{
	mat4 o_matrix = mat4_id();
	f32 c = mcos(angle_radians);
	f32 s = msin(angle_radians);
	o_matrix.data[5] = c;
	o_matrix.data[6] = s;
	o_matrix.data[9] = -s;
	o_matrix.data[10] = c;
	return o_matrix;
}

MINLINE mat4 mat4_eulero_y(f32 angle_radians)
{
	mat4 o_matrix = mat4_id();
	f32 c = mcos(angle_radians);
	f32 s = msin(angle_radians);
	o_matrix.data[0] = c;
	o_matrix.data[2] = -s;
	o_matrix.data[8] = s;
	o_matrix.data[10] = c;
	return o_matrix;
}

MINLINE mat4 mat4_eulero_z(f32 angle_radians)
{
	mat4 o_matrix = mat4_id();
	f32 c = mcos(angle_radians);
	f32 s = msin(angle_radians);
	o_matrix.data[0] = c;
	o_matrix.data[1] = s;
	o_matrix.data[4] = -s;
	o_matrix.data[5] = c;
	return o_matrix;
}

MINLINE mat4 mat4_eulero_xyz(f32 x_radians, f32 y_radians, f32 z_radians)
{
	mat4 rx = mat4_eulero_x(x_radians);
	mat4 ry = mat4_eulero_y(y_radians);
	mat4 rz = mat4_eulero_z(z_radians);
	mat4 o_matrix = mat4_mult(rx, ry);
	o_matrix = mat4_mult(o_matrix, rz);
	return o_matrix;
}

MINLINE vect3 mat4_forward(mat4 matrix) {
	// In OpenGL, Forward is the negative Z-axis (Column 2)

	vect3 forward;
	forward.x = -matrix.data[8];
	forward.y = -matrix.data[9];
	forward.z = -matrix.data[10];
	vect3_normalize(&forward);
	return forward;
}

MINLINE vect3 mat4_backward(mat4 matrix) {
	// In OpenGL, Up is the positive Y-axis (Column 1)
	vect3 up;
	up.x = matrix.data[8];
	up.y = matrix.data[9];
	up.z = matrix.data[10];
	vect3_normalize(&up);
	return up;
}

MINLINE vect3 mat4_up(mat4 matrix) {
	// In OpenGL, Up is the positive Y-axis (Column 1)
	vect3 up;
	up.x = matrix.data[4];
	up.y = matrix.data[5];
	up.z = matrix.data[6];
	vect3_normalize(&up);
	return up;
}

MINLINE vect3 mat4_down(mat4 matrix) {
	// In OpenGL, Down is the negative Y-axis (Column 1)
	vect3 down;
	down.x = -matrix.data[4];
	down.y = -matrix.data[5];
	down.z = -matrix.data[6];
	vect3_normalize(&down);
	return down;
}

MINLINE vect3 mat4_right(mat4 matrix) {
	// In OpenGL, Right is the positive X-axis (Column 0)
	vect3 right;
	right.x = matrix.data[0];
	right.y = matrix.data[1];
	right.z = matrix.data[2];
	vect3_normalize(&right);
	return right;
}

MINLINE vect3 mat4_left(mat4 matrix) {
	// In OpenGL, Left is the negative X-axis (Column 0)
	vect3 left;
	left.x = -matrix.data[0];
	left.y = -matrix.data[1];
	left.z = -matrix.data[2];
	vect3_normalize(&left);
	return left;
}


//extra math functions

MINLINE f32 deg_to_rad(f32 deg)
{
	return deg * M_DEG2RAD_MULTIPLIER;
}

MINLINE f32 rad_to_deg(f32 rad)
{
	return rad * M_RAD2DEG_MULTIPLIER;
}