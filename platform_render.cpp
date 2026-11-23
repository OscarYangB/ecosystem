#include <SDL3/SDL.h>
#include "platform_render.h"
#include <iostream>

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

SDL_Texture* jermaine = nullptr;

void draw_rect_around_point(int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FRect rect = { x, y, w, h };
    SDL_RenderRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void render_entity(float x, float y, double angle) {
    SDL_FRect dst = { x - 25, y - 25, 50, 50 };
    SDL_RenderTextureRotated(renderer, jermaine, nullptr, &dst, (angle / (2 * SDL_PI_F)) * 360.f + 90.f, nullptr, SDL_FLIP_NONE);
}

void start_render() {
    SDL_RenderClear(renderer);
}

void end_render() {
    SDL_RenderPresent(renderer);
}


void load_image(const char* path, SDL_Texture** out_texture) {
    //int count = 0;
    //char** test = SDL_GlobDirectory(".", nullptr, 0, &count);
    //for (int i = 0; i < count; i++) {
    //    std::cout << test[i];
    //}

    SDL_Surface* surface = SDL_LoadPNG(path);
    *out_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
}

void load_assets() {
    load_image("jermaine.png", &jermaine);
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