#include <SDL3/SDL.h>
#include "platform_render.h"

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

void draw_rect_around_point(int x, int y, int w, int h) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    SDL_FRect rect = { x, y, w, h };
    SDL_RenderRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void start_render() {
    SDL_RenderClear(renderer);
}

void end_render() {
    SDL_RenderPresent(renderer);
}

bool start_window() {
    SDL_SetAppMetadata("Ecosystem", "1.0", "");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        return false;
    }

    if (!SDL_CreateWindowAndRenderer("I Love All The Strange People I Don't Know", 1920, 1080, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        return false;
    }

    SDL_SetRenderVSync(renderer, 1);
    SDL_SetRenderLogicalPresentation(renderer, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_DISABLED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    return true;
}