#pragma once

constexpr size_t WIDTH = 1920;
constexpr size_t HEIGHT = 1080;

void draw_rect_around_point(int x, int y, int w, int h);
void start_render();
void end_render();
bool start_window();