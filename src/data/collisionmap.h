#ifndef COLLISIONMAP_H
#define COLLISIONMAP_H

#include "data/datamap.h"
#include <stdint.h>

typedef struct {
	char** data;
	int64_t x;
	int64_t y;
	size_t width;
	size_t height;
} CollisionMap;

CollisionMap* GenerateCollisionMap();
void LoadCollisionChunk(CollisionMap* colmap, Datamap* datamap);
void FreeCollisionMap(CollisionMap* colmap);

#endif
