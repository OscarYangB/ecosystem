#include "ant.h"
#include "physics.h"
#include <cmath>

void Ant::init(float health, float nutrition, float x, float y) {
	body_id = create_physics_entity(x, y, 20.f);
	this->max_health = health;
	this->max_nutrition = nutrition;
	this->nutrition = nutrition;
	this->health = health;
	speed = random_float(8.f, 12.f);
}

void Ant::destroy() {
	destroy_physics_entity(body_id);
}
