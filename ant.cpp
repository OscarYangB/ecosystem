#include "ant.h"
#include "physics.h"
#include <cmath>

void Ant::init(float x, float y) {
	body_id = create_physics_entity(x, y, 20.f);
	health = random_float(60.f, 180.f);
	speed = random_float(8.f, 12.f);
}

void Ant::destroy() {
	destroy_physics_entity(body_id);
}
