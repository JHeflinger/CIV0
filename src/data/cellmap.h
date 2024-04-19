#ifndef CELLMAP_H
#define CELLMAP_H

#include <stdint.h>

typedef struct {
    int64_t x;
    int64_t y;
    size_t width;
    size_t height;
    char** data;
} Cellmap;

void AddCell(Cellmap* map, int64_t x, int64_t y, char id);
void RemoveCell(Cellmap* map, int64_t x, int64_t y);
void ClearCells(Cellmap* map);
char GetCell(Cellmap* map, int64_t x, int64_t y);

#endif