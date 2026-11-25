#pragma once
#include <box2d/box2d.h>

struct Cell;

enum SignalType {
	REPRODUCE = 1 << 1,
	FOOD = 1 << 2,
	DANGER = 1 << 3
};

struct Ant {
	float max_health;
	float max_nutrition;

	float health;
	float nutrition;
	b2BodyId body_id;
	b2Vec2 travel_direction = {1.f, 1.f};
	float direction_timer = 0.f;
	bool dead = false;

	void init(float health, float nutrition, float x, float y);
	void destroy();
};
