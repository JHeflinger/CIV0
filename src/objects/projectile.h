#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "raylib.h"
#include <stdlib.h>
#include <stdint.h>

typedef void (*ProjectileBehavior)(void*); // void* projectile
typedef void (*ProjectileCollisionBehavior)(void*, Vector2, int64_t, int64_t); // void* projectile | Vector2 collision_descriptor | int collision_coordinate_x | int collision_coordinate_y

typedef enum {
	ENABLE_PROJECTILE_NONE          = 1 << 0,
	ENABLE_PROJECTILE_ON_SPAWN      = 1 << 1,
	ENABLE_PROJECTILE_ON_UPDATE     = 1 << 2,
	ENABLE_PROJECTILE_ON_COLLISION  = 1 << 3,
	ENABLE_PROJECTILE_ON_HIT        = 1 << 4,
	ENABLE_PROJECTILE_ON_DEATH      = 1 << 5
} ProjectileFlag;

typedef struct {
	Vector2 prev_position;
	Vector2 position;
	Vector2 velocity;
	Vector2 size;
	float rotation;
	float lifetime;
	ProjectileBehavior on_spawn;
	ProjectileBehavior on_update;
	ProjectileCollisionBehavior on_collision;
	ProjectileBehavior on_hit;
	ProjectileBehavior on_death;
	ProjectileFlag enable_flags;
} Projectile;

Projectile* GenerateDefaultProjectile(Vector2 position, Vector2 velocity);
Projectile* GenerateBouncyProjectile(Vector2 position, Vector2 velocity);

#endif
