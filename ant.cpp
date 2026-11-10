#include "ant.h"
#include "grid.h"

#include <cmath>

Cell* get_cell_from_position(const Vector2& position) {
	size_t x = std::round(position.x);
	size_t y = std::round(position.y);
	return &grid[x][y];
}

Cell* Ant::get_next_cell(const Cell* current_cell) {
	const size_t x = current_cell->x;
	const size_t y = current_cell->y;
	Cell* candidates[] = {get_cell(x+1, y), get_cell(x-1, y), get_cell(x, y-1), get_cell(x, y+1)};

	float max_pheromone = 0.f;
	Cell* best_candidate = nullptr;
	for (Cell* candidate : candidates) {
		if (candidate->type == CellType::EMPTY && candidate->pheromone > max_pheromone) {
			max_pheromone = candidate->pheromone;
			best_candidate = candidate;
		}
	}

	return best_candidate;
}

void Ant::update() {
	Cell* current_cell = get_cell_from_position(position);
	Cell* next_cell = get_next_cell(current_cell);

	if (next_cell != nullptr) {
		direction = Vector2{static_cast<float>(next_cell->x - current_cell->x),
							static_cast<float>(next_cell->y - current_cell->y)};
		velocity = direction;
		position += velocity;
	}
}
