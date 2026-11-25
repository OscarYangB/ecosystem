#pragma once

#include <box2d/box2d.h>

extern b2WorldId world_id;

constexpr b2Vec2 MONUMENT_DIMENSIONS[] = {
    { 200,200 },
    { 200,100 },
    { 100,200 },
    { 150,150 },
    { 250,150 },
    { 250,300 },
    { 200,500 },
    { 400,300 },
    { 500,400 },
    { 600,300 },
};

void init_physics();
b2BodyId create_static_physics_entity(float x, float y, float rotation, float size);
b2BodyId create_physics_entity(float x, float y, float size);
void destroy_physics_entity(b2BodyId body);
void update_physics(double deltatime);
void move_monument(int id, b2Vec2 position);
void disable_monument(int id);
bool is_monument_enabled(int id);
float random_float(float min_inclusive, float min_exclusive);

template<typename T>
T exponential_decay(T a, T b, float decay, float dt) {
    return b + (a - b) * std::exp(-decay * dt);
}

inline float degtorad(float angle) {
    return (angle / 360.f) * 2 * 3.14f;
}