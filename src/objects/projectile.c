#include "projectile.h"
#include "core/logger.h"
#include "data/gconfig.h"
#include <stdlib.h>

void GenericNotifyFunction(void* args) {
	LOG_INFO("Generic notification!");
}

void BounceProjectile(void* args, Vector2 collisiontype, int64_t col_coord_x, int64_t col_coord_y) {
	Projectile* projectile = (Projectile*)args;
	if (collisiontype.x != 0) {
		projectile->velocity.x *= -1.0f;
		projectile->position.x = collisiontype.x > 0 ? (float)(col_coord_x * CELLSIZE) - projectile->size.x : (float)((1 + col_coord_x) * CELLSIZE);
	}
	if (collisiontype.y != 0) {
		projectile->velocity.y *= -1.0f;
		projectile->position.y = collisiontype.y > 0 ? (float)(col_coord_y * CELLSIZE) - projectile->size.y : (float)((1 + col_coord_y) * CELLSIZE);
	}
}

Projectile* GenerateDefaultProjectile(Vector2 position, Vector2 velocity) {
	Projectile* projectile = calloc(1, sizeof(Projectile));
	projectile->position = position;
	projectile->prev_position = position;
	projectile->velocity = velocity;
	projectile->size.x = 10.0f;
	projectile->size.y = 10.0f;
	projectile->rotation = 0.0f;
	projectile->lifetime = 5.0f;
	projectile->enable_flags = ENABLE_PROJECTILE_NONE;
	return projectile;
}

Projectile* GenerateBouncyProjectile(Vector2 position, Vector2 velocity) {
	Projectile* projectile = GenerateDefaultProjectile(position, velocity);
	projectile->on_collision = BounceProjectile;
	projectile->enable_flags = ENABLE_PROJECTILE_ON_COLLISION;
	return projectile;
}