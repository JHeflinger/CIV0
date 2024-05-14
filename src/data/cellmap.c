#include "cellmap.h"
#include "core/logger.h"

void AddCell(Cellmap* map, int64_t x, int64_t y, char id) {
    if (x < -1*map->bounds_w ||
        x > map->bounds_w ||
        y < -1*map->bounds_h ||
        y > map->bounds_h) {
        return;
    } else {
        if (id == '\0') LOG_FATAL("DO NOT ADD A CELL WITH VALUE \\0 - USE RemoveCell INSTEAD");
        if (map->data == NULL || 
            (map->width == 0 && map->height == 0)) {
            map->width = 1;
            map->height = 1;
            map->x = x;
            map->y = y;
            map->data = calloc(1, sizeof(char*));
            map->data[0] = calloc(1, sizeof(char));
            map->data[0][0] = id;
        } else {
            int64_t datax = x - map->x;
            int64_t datay = y - map->y;
            if (datax < 0 || datay < 0) {
                size_t new_w = datax < 0 ? map->width * 2 : map->width;
                size_t new_h = datay < 0 ? map->height * 2 : map->height;
                uint8_t** new_data = calloc(new_w, sizeof(uint8_t*));
                for (size_t i = 0; i < new_w; i++) {
                    new_data[i] = calloc(new_h, sizeof(uint8_t));
                    for (size_t j = 0; j < new_h; j++) new_data[i][j] = '\0';
                }
                for (size_t i = 0; i < map->width; i++)
                    for (size_t j = 0; j < map->height; j++)
                        new_data[new_w - map->width + i][new_h - map->height + j] = map->data[i][j];
                for (size_t i = 0; i < map->width; i++) free(map->data[i]);
                free(map->data);
                map->x -= new_w - map->width;
                map->y -= new_h - map->height;
                map->width = new_w;
                map->height = new_h;
                map->data = new_data;
                AddCell(map, x, y, id);
            } else if (datax >= map->width || datay >= map->height) {
                size_t new_w = datax >= map->width ? map->width * 2 : map->width;
                size_t new_h = datay >= map->height ? map->height * 2 : map->height;
                uint8_t** new_data = calloc(new_w, sizeof(uint8_t*));
                for (size_t i = 0; i < new_w; i++) {
                    new_data[i] = calloc(new_h, sizeof(uint8_t));
                    for (size_t j = 0; j < new_h; j++) new_data[i][j] = '\0';
                }
                for (size_t i = 0; i < map->width; i++)
                    for (size_t j = 0; j < map->height; j++)
                        new_data[i][j] = map->data[i][j];
                for (size_t i = 0; i < map->width; i++) free(map->data[i]);
                free(map->data);
                map->width = new_w;
                map->height = new_h;
                map->data = new_data;
                AddCell(map, x, y, id);
            } else {
                map->data[(size_t)datax][(size_t)datay] = id;
            }
        }
    }
}

void RemoveCell(Cellmap* map, int64_t x, int64_t y) {
    map->data[(size_t)(x - map->x)][(size_t)(y - map->y)] = '\0';
}

void ClearCells(Cellmap* map) {
    for (size_t i = 0; i < (size_t)map->width; i++)
        free(map->data[i]);
    free(map->data);
    map->data = NULL;
    map->x = 0;
    map->y = 0;
    map->width = 0;
    map->height = 0;
}

char GetCell(Cellmap* map, int64_t x, int64_t y) {
    if (x < -1*map->bounds_w) return GetCell(map, x + 2*map->bounds_w, y);
    if (x > map->bounds_w) return GetCell(map, x - 2*map->bounds_w, y);
    if (y < -1*map->bounds_h) return GetCell(map, x, y + 2*map->bounds_h);
    if (y > map->bounds_h) return GetCell(map, x, y - 2*map->bounds_h);
    return map->data[(size_t)(x - map->x)][(size_t)(y - map->y)];
}

void SetMapBounds(Cellmap* map, int w, int h) {
    map->bounds_w = w;
    map->bounds_h = h;
}