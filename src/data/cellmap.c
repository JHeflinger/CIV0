#include "cellmap.h"
#include "core/logger.h"

void AddCell(Cellmap* map, int64_t x, int64_t y, char id) {
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
        Coordinate coord;
        coord.x = 0;
        coord.y = 0;
        ARRLIST_Coordinate_add(&(map->active), coord);
    } else {
        int64_t datax = x - map->x;
        int64_t datay = y - map->y;
        if (datax < 0 || datay < 0) {
            size_t new_w = datax < 0 ? map->width * 2 : map->width;
            size_t new_h = datay < 0 ? map->height * 2 : map->height;
            char** new_data = calloc(new_w, sizeof(char*));
            for (size_t i = 0; i < new_w; i++) {
                new_data[i] = calloc(new_h, sizeof(char));
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
            char** new_data = calloc(new_w, sizeof(char*));
            for (size_t i = 0; i < new_w; i++) {
                new_data[i] = calloc(new_h, sizeof(char));
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
            if (map->data[(size_t)datax][(size_t)datay] != '\0') {
                for (size_t i = 0; i < map->active.size; i++) {
                    if (map->active.data[i].x == datax && map->active.data[i].y == datay) {
                        ARRLIST_Coordinate_remove(&(map->active), i);
                        break;
                    }
                }
            }
            map->data[(size_t)datax][(size_t)datay] = id;
            Coordinate coord;
            coord.x = x;
            coord.y = y;
            ARRLIST_Coordinate_add(&(map->active), coord);
        }
    }
}

void RemoveCell(Cellmap* map, int64_t x, int64_t y) {
    int64_t datax = x - map->x;
    int64_t datay = y - map->y;
    if (map->data[(size_t)(datax)][(size_t)(datay)] != '\0') {
        for (size_t i = 0; i < map->active.size; i++) {
            if (map->active.data[i].x == datax && map->active.data[i].y == datay) {
                ARRLIST_Coordinate_remove(&(map->active), i);
                break;
            }
        }
        map->data[(size_t)(datax)][(size_t)(datay)] = '\0';
    }
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
    ARRLIST_Coordinate_clear(&(map->active));
}

char GetCell(Cellmap* map, int64_t x, int64_t y) {
    return map->data[(size_t)(x - map->x)][(size_t)(y - map->y)];
}
