#pragma once

#include "vector2.h"

struct Cell;

enum class Mode {
	SCOUT,
	FOLLOW,
};

struct Ant {
	float max_health;
	float max_nutrition;
	float max_speed;

	Mode mode = Mode::SCOUT;
	float health;
	float nutrition;
	Vector2 velocity;
	Vector2 position;
	Vector2 direction;

	Cell* get_next_cell(const Cell* current_cell);
	void update();
};
