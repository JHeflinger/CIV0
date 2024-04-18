#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include "core/logger.h"
#include "easynet.h"
#include "objects/projectile.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DECLARE_ARRLIST(T) \
typedef struct { \
	size_t size; \
	size_t maxsize; \
	T* data; \
} ARRLIST_##T; \
\
void ARRLIST_##T##_add(ARRLIST_##T* list, T element); \
void ARRLIST_##T##_remove(ARRLIST_##T* list, size_t index); \
T ARRLIST_##T##_get(ARRLIST_##T* list, size_t index); \
void ARRLIST_##T##_clear(ARRLIST_##T* list);

#define IMPL_ARRLIST(T) \
void ARRLIST_##T##_add(ARRLIST_##T* list, T element) { \
	if (list->maxsize == 0) { \
		list->data = (T*)calloc(1, sizeof(T)); \
		list->size = 1; \
		list->maxsize = 1; \
		list->data[0] = element; \
	} else if (list->size < list->maxsize) { \
		list->data[list->size] = element; \
		list->size++; \
	} else { \
		list->maxsize *= 2; \
		T* newdata = (T*)calloc(list->maxsize, sizeof(T)); \
		memcpy(newdata, list->data, sizeof(T)*list->size); \
		free(list->data); \
		list->data = newdata; \
		list->data[list->size] = element; \
		list->size++; \
	} \
} \
\
void ARRLIST_##T##_remove(ARRLIST_##T* list, size_t index) { \
	if (index >= list->size) \
		LOG_FATAL("Invalid arraylist index to remove"); \
	if (index == list->size - 1)  { \
		list->size--; \
		return; \
	} \
	for (size_t i = index; i < list->size - 1; i++) \
		list->data[i] = list->data[i + 1]; \
	list->size--; \
} \
\
T ARRLIST_##T##_get(ARRLIST_##T* list, size_t index) { \
	if (index >= list->size) \
		LOG_FATAL("Invalid arraylist index to get"); \
	return list->data[index]; \
} \
\
void ARRLIST_##T##_clear(ARRLIST_##T* list) { \
	free(list->data); \
	list->size = 0; \
	list->maxsize = 0; \
}

#define DECLARE_ARRLIST_NAMED(name, T) \
typedef struct { \
	size_t size; \
	size_t maxsize; \
	T* data; \
} ARRLIST_##name; \
\
void ARRLIST_##name##_add(ARRLIST_##name* list, T element); \
void ARRLIST_##name##_remove(ARRLIST_##name* list, size_t index); \
T ARRLIST_##name##_get(ARRLIST_##name* list, size_t index); \
void ARRLIST_##name##_clear(ARRLIST_##name* list);

#define IMPL_ARRLIST_NAMED(name, T) \
void ARRLIST_##name##_add(ARRLIST_##name* list, T element) { \
	if (list->maxsize == 0) { \
		list->data = (T*)calloc(1, sizeof(T)); \
		list->size = 1; \
		list->maxsize = 1; \
		list->data[0] = element; \
	} else if (list->size < list->maxsize) { \
		list->data[list->size] = element; \
		list->size++; \
	} else { \
		list->maxsize *= 2; \
		T* newdata = (T*)calloc(list->maxsize, sizeof(T)); \
		memcpy(newdata, list->data, sizeof(T)*list->size); \
		free(list->data); \
		list->data = newdata; \
		list->data[list->size] = element; \
		list->size++; \
	} \
} \
\
void ARRLIST_##name##_remove(ARRLIST_##name* list, size_t index) { \
	if (index >= list->size) \
		LOG_FATAL("Invalid arraylist index to remove"); \
	if (index == list->size - 1)  { \
		list->size--; \
		return; \
	} \
	for (size_t i = index; i < list->size - 1; i++) \
		list->data[i] = list->data[i + 1]; \
	list->size--; \
} \
\
T ARRLIST_##name##_get(ARRLIST_##name* list, size_t index) { \
	if (index >= list->size) \
		LOG_FATAL("Invalid arraylist index to get"); \
	return list->data[index]; \
} \
\
void ARRLIST_##name##_clear(ARRLIST_##name* list) { \
	free(list->data); \
	list->size = 0; \
	list->maxsize = 0; \
}

// declare custom arrlists here!
DECLARE_ARRLIST(int);
DECLARE_ARRLIST(char);
DECLARE_ARRLIST(float);
DECLARE_ARRLIST(double);
DECLARE_ARRLIST(uint8_t);
DECLARE_ARRLIST(uint16_t);
DECLARE_ARRLIST(uint32_t);
DECLARE_ARRLIST(uint64_t);
DECLARE_ARRLIST(int8_t);
DECLARE_ARRLIST(int16_t);
DECLARE_ARRLIST(int32_t);
DECLARE_ARRLIST(int64_t);
DECLARE_ARRLIST(EZN_SOCKET);
DECLARE_ARRLIST_NAMED(ProjectilePtr, Projectile*);

#endif
