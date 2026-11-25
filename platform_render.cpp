#include "platform_render.h"
#include <iostream>

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

SDL_Texture* jermaine = nullptr;
SDL_Texture* tree_1 = nullptr;
SDL_Texture* tree_2 = nullptr;
SDL_Texture* tree_3 = nullptr;
SDL_Texture* circle_1 = nullptr;
SDL_Texture* circle_2 = nullptr;
SDL_Texture* circle_3 = nullptr;

void draw_rect_around_point(int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FRect rect = { x, y, w, h };
    SDL_RenderRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void render_entity(float x, float y, double angle, float w, float h, SDL_Texture* texture) {
    const float half_w = w / 2.f;
    const float half_h = h / 2.f;
    SDL_FRect dst = { x - half_w, y - half_h, w, h };
    SDL_RenderTextureRotated(renderer, texture, nullptr, &dst, (angle / (2 * SDL_PI_F)) * 360.f + 90.f, nullptr, SDL_FLIP_NONE);
}

SDL_Texture* get_tree(int index) {
    switch (index) {
    case 0:
        return tree_1;
    case 1:
        return tree_2;
    case 2:
        return tree_3;
    default:
        return tree_1;
    }
}

void render_tree(float x, float y, int index) {
    SDL_FRect dst = { x, y, 328, 472 };
    SDL_RenderTextureRotated(renderer, get_tree(index), nullptr, &dst, 0, nullptr, SDL_FLIP_NONE);
}

void start_render() {
    SDL_RenderClear(renderer);
}

void end_render() {
    SDL_RenderPresent(renderer);
}


void load_image(const char* path, SDL_Texture** out_texture) {
    SDL_Surface* surface = SDL_LoadPNG(path);
    *out_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
}

void load_assets() {
    load_image("jermaine.png", &jermaine);
    load_image("tree_1.png", &tree_1);
    load_image("tree_2.png", &tree_2);
    load_image("tree_3.png", &tree_3);
    load_image("circle_1.png", &circle_1);
    load_image("circle_2.png", &circle_2);
    load_image("circle_3.png", &circle_3);
}

bool start_window() {
    SDL_SetAppMetadata("Ecosystem", "1.0", "");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return false;
    }

    if (!SDL_CreateWindowAndRenderer("Ecosystem", 1920, 1080, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        return false;
    }

    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_DISABLED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    load_assets();

    return true;
}

void DrawLine(float x1, float y1, float x2, float y2) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderLine(renderer, x1, y1, x2, y2);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}