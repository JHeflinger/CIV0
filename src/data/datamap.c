#include "datamap.h"
#include "core/logger.h"
#include <stdlib.h>
#include <string.h>

Datamap* GenerateDatamap(char* str_data) {
	Datamap* datamap = calloc(1, sizeof(Datamap));
	datamap->size = strlen(str_data);
	datamap->data = calloc(1, datamap->size);
	strcpy(datamap->data, str_data);

	// parse setup
	size_t word_start = 0;
	size_t word_end = 0;
	char* linestr;

	// parse type
	word_end = word_start;
	while(datamap->data[word_end] != '\0' && datamap->data[word_end] != '\n')
		word_end++;
	linestr = calloc(1, 2 + word_end - word_start);
	memcpy(linestr, str_data + word_start, word_end - word_start);
	linestr[word_end - word_start] = '\0';
	if (strcmp(linestr, "#TYPE COLLISION") == 0) {
		datamap->type = MAP_COLLISION;
	} else if (strcmp(linestr, "#TYPE NONE") == 0) {
		datamap->type = MAP_NONE;
	} else {
		LOG_FATAL("A map with an invalid type was attempted to be loaded");
	}
	free(linestr);

	return datamap;
}

void FreeDatamap(Datamap* map) {
	free(map->data);
	free(map);
}
