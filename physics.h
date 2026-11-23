#pragma once

void init_physics();
b2BodyId create_physics_entity(float x, float y, float size);
void destroy_physics_entity(b2BodyId body);
void update_physics(double deltatime);