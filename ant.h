#pragma once
#include <box2d/box2d.h>

struct Cell;

enum class Mode {
	SCOUT,
	FOLLOW,
};

struct Ant {
	float max_health;
	float max_nutrition;

	Mode mode = Mode::SCOUT;
	float health;
	float nutrition;
	b2BodyId body_id;
	b2Vec2 force_direction = {1.f, 1.f};
	Cell* get_next_cell(const Cell* current_cell);
	Ant(float health, float nutrition, float x, float y);
	~Ant();
	void update();
};
