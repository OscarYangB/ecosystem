#pragma once

#include <SDL3/SDL.h>

constexpr size_t WIDTH = 1920;
constexpr size_t HEIGHT = 1080;

extern SDL_Texture* jermaine;
extern SDL_Texture* tree_1;
extern SDL_Texture* tree_2;
extern SDL_Texture* tree_3;
extern SDL_Texture* circle_1;
extern SDL_Texture* circle_2;
extern SDL_Texture* circle_3;
extern SDL_Texture* food_1;
extern SDL_Texture* food_2;
extern SDL_Texture* food_3;
extern SDL_Texture* food_4;
extern SDL_Texture* code_1;
extern SDL_Texture* code_2;
extern SDL_Texture* code_3;
extern SDL_Texture* code_4;

void draw_rect_around_point(int x, int y, int w, int h);
void start_render();
void end_render();
bool start_window();
void render_entity(float x, float y, double angle, float w, float h, SDL_Texture* texture);
void render_tree(float x, float y, int index);
void DrawLine(float x1, float y1, float x2, float y2);
void render_code(float x, float y, float w, float h, SDL_Texture* texture);
int get_screen_width();
int get_screen_height();
void draw_white();
void draw_black();