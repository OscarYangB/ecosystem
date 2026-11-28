#include <vector>
#include "physics.h"
#include <random>

b2WorldId world_id;
std::vector<b2BodyId> monument_bodies = {};

using RandomGeneratorType = std::mt19937;
RandomGeneratorType random_generator;

float random_float(float min_inclusive, float min_exclusive) {
	std::uniform_real_distribution<float> distribution(min_inclusive, min_exclusive);
	return distribution(random_generator);
}

int random_integer(int min_inclusive, int max_inclusive) {
	std::uniform_int_distribution<int> distribution(min_inclusive, max_inclusive);
	return distribution(random_generator);
}

b2BodyId create_static_physics_entity(float x, float y, float rotation, float size) {
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position = b2Pos{ x, y };
	bodyDef.rotation = b2MakeRot(rotation);
	b2BodyId bodyId = b2CreateBody(world_id, &bodyDef);
	b2Circle circle = {};
	circle.center = { 0.f,0.f };
	circle.radius = size;
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.enableSensorEvents = true;
	b2CreateCircleShape(bodyId, &shapeDef, &circle);
	return bodyId;
}

static b2BodyId create_monument(float w, float h) {
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position = b2Pos{ 0.f, 0.f };
	bodyDef.rotation = b2MakeRot(0.f);
	b2BodyId bodyId = b2CreateBody(world_id, &bodyDef);
	b2Body_Disable(bodyId);
	b2Polygon box = b2MakeBox(w/2.f, h/2.f);
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(bodyId, &shapeDef, &box);
	return bodyId;
}

static void init_monuments() {
	for (const b2Vec2& dimension : MONUMENT_DIMENSIONS) {
		monument_bodies.emplace_back(create_monument(dimension.x, dimension.y));
	}
}

static void make_boundary(float x, float y, float rotation) {
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_staticBody;
	bodyDef.position = b2Pos{ x, y };
	bodyDef.rotation = b2MakeRot(rotation);
	b2BodyId bodyId = b2CreateBody(world_id, &bodyDef);
	b2Polygon box = b2MakeBox(5000.f, 30.f);
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	b2CreatePolygonShape(bodyId, &shapeDef, &box);
}

void init_boundaries() {
	make_boundary(0.f, 0.f, 0.f);
	make_boundary(0.f, 0.f, 3.14f / 2.f);
	make_boundary(1920.f, 0.f, 3.14f / 2.f);
	make_boundary(1920.f, 1080.f, 0.f);
}

void init_physics() {
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = { 0.f, 0.f };
	world_id = b2CreateWorld(&worldDef);
	init_boundaries();
	init_monuments();

	std::random_device device{};
	random_generator = std::mt19937{ device() };
}

b2BodyId create_physics_entity(float x, float y, float size, bool is_sensor) {
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Pos{ x, y };
	bodyDef.rotation = b2MakeRot(0.f);
	bodyDef.linearDamping = 0.0f;
	bodyDef.angularDamping = 0.8f;
	bodyDef.gravityScale = 0.f;
	bodyDef.enableSleep = false;
	b2BodyId bodyId = b2CreateBody(world_id, &bodyDef);
	b2Circle circle = {};
	circle.center = { 0.f,0.f };
	circle.radius = size;
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.isSensor = is_sensor;
	shapeDef.enableSensorEvents = is_sensor;
	shapeDef.density = 0.6f;
	shapeDef.material.friction = 500.f;
	b2CreateCircleShape(bodyId, &shapeDef, &circle);
	b2Body_EnableContactEvents(bodyId, true);
	return bodyId;
}

void destroy_physics_entity(b2BodyId body) {
	b2DestroyBody(body);
}

void update_physics(double deltatime) {
	b2World_Step(world_id, deltatime, 4);
}

void move_monument(int id, b2Vec2 position) {
	if (!b2Body_IsEnabled(monument_bodies[id])) {
		b2Body_Enable(monument_bodies[id]);
	}

	if (b2Distance(position, b2Body_GetPosition(monument_bodies[id])) < 10.f) {
		return;
	}
	b2Body_SetTransform(monument_bodies[id], position, b2MakeRot(0.f));
}

void disable_monument(int id) {
	b2Body_Disable(monument_bodies[id]);
}

bool is_monument_enabled(int id)
{
	return b2Body_IsEnabled(monument_bodies[id]);
}
