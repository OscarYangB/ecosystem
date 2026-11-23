#include <box2d/box2d.h>
#include <vector>
#include "physics.h"

b2WorldId world_id;
std::vector<b2BodyId> bodies = {};

void init_physics() {
	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = { 0.f, 0.f };
	world_id = b2CreateWorld(&worldDef);
}

b2BodyId create_physics_entity(float x, float y, float size) {
	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = b2Pos{ x, y };
	bodyDef.rotation = b2MakeRot(0.f);
	bodyDef.linearDamping = 0.f;
	bodyDef.angularDamping = 0.1f;
	bodyDef.gravityScale = 0.f;
	bodyDef.enableSleep = false;
	b2BodyId bodyId = b2CreateBody(world_id, &bodyDef);
	b2Circle circle = {};
	circle.center = { 0.f,0.f };
	circle.radius = size;
	b2ShapeDef shapeDef = b2DefaultShapeDef();
	shapeDef.density = 1.0f;
	shapeDef.material.friction = 0.6f;
	b2CreateCircleShape(bodyId, &shapeDef, &circle);
	bodies.push_back(bodyId);
	bool test = b2Body_IsValid(bodyId);
	return bodyId;
}

void destroy_physics_entity(b2BodyId body) {
	b2DestroyBody(body);
	auto remove_start = std::remove_if(bodies.begin(), bodies.end(), [](b2BodyId current_body) { return !b2Body_IsValid(current_body); });
	bodies.erase(remove_start, bodies.end());
}

void update_physics(double deltatime) {
	b2World_Step(world_id, deltatime, 4);

	for (const b2BodyId id : bodies) {
		//b2Body_ApplyForceToCenter(id, b2Vec2{ -100.0f, 100.0f }, true);
	}
}