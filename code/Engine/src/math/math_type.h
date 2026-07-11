#pragma once
#include "defines.hpp"


typedef union vect2
{
	f32 elements[2];
		struct 
		{
			union 
			{
				f32 x, r, s, u;
			};
			union
			{
				f32 y, g, t, v;
			};

		};
	
}vect2;

typedef union vect3
{
	union
	{
		f32 elements[3];
		struct
		{
			union
			{
				f32 x, r, s, u;
			};
			union
			{
				f32 y, g, t, v;
			};
			union
			{
				f32 z, b, p, w;
			};
		};
	};
}vect3;



typedef union vect4
{
#if defined(KUSE_SIMD)
	alignas(16) __m128 data;
#endif
	alignas(16) f32 elements[4];
	union
	{
		struct
		{
			//array of 4 so for example now we have color and transparency [r,g,b,a]
			union
			{
				f32 x, r, s;
			};
			union
			{
				f32 y, g, t;
			};
			union
			{
				f32 z, b, p;
			};
			//Positions vs. Directions
			union
			{
				f32 w, a, q;
			};
		};
	};
}vect4;

typedef vect4 quat;

typedef union mat4
{
	alignas(16) f32 data[16];
#if defined (KUSE_SIMD)
	alignas(16) vect4 rows[4];
#endif
}mat4;


//useful for buildings, terrains, stairs on a 3d rendering
typedef struct vertex_3d
{
	vect3 position; // 3D position of the vertex
	vect2 texture; //texture coordinates
	vect3 normal; //normal vector for lighting calculations
}vertex_3d;


//structure for 2D vertices, useful for sprites and 2D rendering
typedef struct vertex_2d
{
	vect2 position; 
	vect2 texcoord; //texture coordinates spritesheet
}vertex_2d;