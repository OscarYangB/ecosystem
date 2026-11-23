#include "grid.h"

Cell grid[GRID_WIDTH][GRID_HEIGHT] = {};

void init_grid() {
	for (size_t i = 0; i < GRID_WIDTH; i++) {
		for (size_t j = 0; j < GRID_HEIGHT; j++) {
			grid[i][j].x = i;
			grid[i][j].y = j;
		}
	}
}

Cell* get_cell(size_t x, size_t y) {
	if (x >= GRID_WIDTH || y >= GRID_HEIGHT || x < 0 || y < 0) {
		return nullptr;
	}

	return &grid[x][y];
}
