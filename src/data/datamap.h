#ifndef DATAMAP_H 
#define DATAMAP_H

#include <string.h>

typedef enum {
	MAP_NONE,
	MAP_COLLISION,
} MapType;

typedef struct {
	char* data;
	size_t size;
	MapType type;
} Datamap;

Datamap* GenerateDatamap(char* str_data);
void FreeDatamap(Datamap* map);

#endif
