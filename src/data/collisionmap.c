#include "collisionmap.h"
#include "core/logger.h"
#include <stdio.h>
#include <stdlib.h>

CollisionMap* GenerateCollisionMap() {
	CollisionMap* colmap = calloc(1, sizeof(CollisionMap));
	colmap->data = NULL;
	colmap->x = 0;
	colmap->y = 0;
	colmap->width = 0;
	colmap->height = 0;
	return colmap;
}

void LoadCollisionChunk(CollisionMap* colmap, Datamap* datamap) {
	if (datamap->type != MAP_COLLISION) LOG_FATAL("Unable to load a non-collision datamap as a collision chunk");
	
	size_t line_start = 0;
	size_t line_end = 0;
	size_t word_start = 0;
	size_t word_end = 0;
	size_t ind = 0;
	size_t dataw = 0;
	size_t datah = 0;
	int currline = 0;
	int currword = 0;
	char* linestr = NULL;
	char* wordstr = NULL;

	#define READLINE()  line_end = line_start;\
						if (linestr != NULL) free(linestr);\
						while(datamap->data[line_end] != '\0' && datamap->data[line_end] != '\n')\
							line_end++;\
						linestr = calloc(1, 2 + line_end - line_start);\
						memcpy(linestr, datamap->data + line_start, line_end - line_start);\
						linestr[line_end - line_start] = '\0';\
						line_start = line_end + 1;\
						word_start = 0;\
						currline++;\
						currword = 0;

	#define READWORD()  word_end = word_start;\
						if (wordstr != NULL) free(wordstr);\
						while(linestr[word_end] != '\0' && linestr[word_end] != '\n' && linestr[word_end] != ' ')\
							word_end++;\
						wordstr = calloc(1, 2 + word_end - word_start);\
						memcpy(wordstr, linestr + word_start, word_end - word_start);\
						wordstr[word_end - word_start] = '\0';\
						word_start = word_end + 1;\
						currword++;

	#define NUMWORD()   ind = 0;\
						if (wordstr[ind] == '-' && wordstr[ind+1] != '\0') ind++;\
						while(wordstr[ind] != '\0') {\
							if (wordstr[ind] < '0' || wordstr[ind] > '9') LOG_FATAL("Expected a number value - line %d | word %d", currline, currword);\
							ind++;\
						}

	#define VALIDDATA() dataw = 0;\
						datah = 0;\
						ind = 0;\
						while(datamap->data[ind] != '\0') {\
							if (datah == 3 && datamap->data[ind] != '\n') dataw++;\
							if (datamap->data[ind] == '\n') datah++;\
							ind++;\
						}\
						datah -= 3;

	if (colmap->data == NULL) { // new map from scratch
		// ignore #TYPE as that has already been handled
		READLINE();

		// read origin
		READLINE();
		READWORD();
		if (strcmp(wordstr, "#ORIGIN") != 0)
			LOG_FATAL("Invalid collision map format - #ORIGIN was not found on line %d", currline);
		READWORD();
		NUMWORD();
		colmap->x = (int64_t)strtol(wordstr, NULL, 10);
		READWORD();
		NUMWORD();
		colmap->y = (int64_t)strtol(wordstr, NULL, 10);
		
		// read data
		READLINE();
		if (strcmp(linestr, "#DATA") != 0)
			LOG_FATAL("Invalid collision map format - #DATA was not found on line %d", currline);
		VALIDDATA();
		colmap->data = calloc(dataw, sizeof(char*));
		for (int i = 0; i < dataw; i++) colmap->data[i] = calloc(datah, sizeof(char));
		for (int y = 0; y < datah; y++) {
			READLINE();
			if (strlen(linestr) != dataw) LOG_FATAL("Non-uniform chunk size detected on line %d", currline);
			for (int x = 0; x < dataw; x++) {
				colmap->data[x][y] = linestr[x];
			} 
		}
		colmap->width = dataw;
		colmap->height = datah;
	} else { // adding to already existing map
		// ignore #TYPE as that has already been handled
		READLINE();

		// read origin
		int64_t newx, newy;
		READLINE();
		READWORD();
		if (strcmp(wordstr, "#ORIGIN") != 0)
			LOG_FATAL("Invalid collision map format - #ORIGIN was not found on line %d", currline);
		READWORD();
		NUMWORD();
		newx = (int64_t)strtol(wordstr, NULL, 10);
		READWORD();
		NUMWORD();
		newy = (int64_t)strtol(wordstr, NULL, 10);

		// read data
		READLINE();
		if (strcmp(linestr, "#DATA") != 0)
			LOG_FATAL("Invalid collision map format - #DATA was not found on line %d", currline);
		VALIDDATA();
		int64_t finalx = newx < colmap->x ? newx : colmap->x;
		int64_t finaly = newy < colmap->y ? newy : colmap->y;
		size_t finalw = newx + dataw > colmap->x + colmap->width ? newx + dataw - finalx : colmap->x + colmap->width - finalx;
		size_t finalh = newy + datah > colmap->y + colmap->height ? newy + datah - finaly : colmap->y + colmap->height - finaly;
		char** newdata = calloc(finalw, sizeof(char*));
		for (int i = 0; i < finalw; i++) {
			newdata[i] = calloc(finalh, sizeof(char));
			memset(newdata[i], ' ', finalh);
		}
		for (int y = 0; y < colmap->height; y++)
			for (int x = 0; x < colmap->width; x++)
				newdata[x + colmap->x + finalx][y + colmap->y + finaly] = colmap->data[x][y];
		for (int y = 0; y < datah; y++) {
			READLINE();
			if (strlen(linestr) != dataw) LOG_FATAL("Non-uniform chunk size detected on line %d", currline);
			for (int x = 0; x < dataw; x++) {
				newdata[x + newx + finalx][y + newy + finaly] = linestr[x];
			}
		}
		colmap->width = finalw;
		colmap->height = finalh;
		colmap->x = finalx;
		colmap->y = finaly;
		free(colmap->data);
		colmap->data = newdata;
	}

	free(linestr);
	free(wordstr);
	#undef READLINE
	#undef READWORD
	#undef NUMWORD
	#undef VALIDDATA
}

void FreeCollisionMap(CollisionMap* colmap) {
	for (int i = 0; i < colmap->width; i++) {
		free(colmap->data[i]);
	}
	free(colmap->data);
	free(colmap);
}
