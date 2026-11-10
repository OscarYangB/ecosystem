#include "grid.h"

Cell grid[WIDTH][HEIGHT] = {};

void init_grid() {
	for (size_t i = 0; i < WIDTH; i++) {
		for (size_t j = 0; j < HEIGHT; j++) {
			grid[i][j].x = i;
			grid[i][j].y = j;
		}
	}
}

Cell* get_cell(size_t x, size_t y) {
	if (x >= WIDTH || y >= HEIGHT || x < 0 || y < 0) {
		return nullptr;
	}

	return &grid[x][y];
}
