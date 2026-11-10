#pragma once

#include <cstddef>

enum class CellType {
	EMPTY,
	BLUE,
	RED,
	GREEN
};

struct Cell {
	size_t x;
	size_t y;

	CellType type = CellType::EMPTY;
	float pheromone = 0.f;
};

constexpr size_t WIDTH = 960;
constexpr size_t HEIGHT = 540;
extern Cell grid[WIDTH][HEIGHT];

Cell* get_cell(size_t x, size_t y);
void init_grid();
