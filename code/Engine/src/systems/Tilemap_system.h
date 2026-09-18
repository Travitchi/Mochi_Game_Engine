#pragma once
#include "defines.hpp"
#include "resource_types.h"
#include "resource_systems.h"
#include "sprite_manager_system.h"


typedef struct tilemap
{
	u32 width;
	u32 depth;
	u32 height;
	f32 tilemap_size;
	vect3 origin;
	i32* tiles;

	struct sprite_sheet* tileset;
	struct material* map_mat;
	struct geometry* geom;

}tilemap;

typedef struct tile
{
	b8 tile_id;
	struct sprite* tile_sprite;

	
};