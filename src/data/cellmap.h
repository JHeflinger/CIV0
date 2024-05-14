#ifndef CELLMAP_H
#define CELLMAP_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    int bounds_w;
    int bounds_h;
    int64_t x;
    int64_t y;
    size_t width;
    size_t height;
    uint8_t** data;
} Cellmap;

void AddCell(Cellmap* map, int64_t x, int64_t y, char id);
void RemoveCell(Cellmap* map, int64_t x, int64_t y);
void ClearCells(Cellmap* map);
char GetCell(Cellmap* map, int64_t x, int64_t y);
void SetMapBounds(Cellmap* map, int w, int h);

#endif
