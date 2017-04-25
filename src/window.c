#include "window.h"
#include "log.h"

struct window * window_create(int width, int height) {

    struct window *window = NULL;

    if (width <= 0 || height <= 0) {
        LOGE(NO_DUMP_ERRNO, "Width or height is invaild.\n");
        goto err_return;
    }

    window = malloc(sizeof(struct window));
    if (window == NULL) {
        LOGE(NO_DUMP_ERRNO, "Out of memory\n");
        goto err_return;
    }

    if (SDL_Init(SDL_INIT_VIDEO)) {
        LOGE(NO_DUMP_ERRNO, "%s", SDL_GetError());
        goto free_window;
    }

    window->sdl_window = SDL_CreateWindow("Tiny Camera",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height, SDL_WINDOW_SHOWN);

    if (window->sdl_window == NULL) {
        LOGE(NO_DUMP_ERRNO, "%s", SDL_GetError());
        goto free_window;
    }

    window->sdl_renderer = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_SOFTWARE);
    if (window->sdl_renderer == NULL) {
        LOGE(NO_DUMP_ERRNO, "%s", SDL_GetError());
        goto free_sdl_window;
    }
    window->sdl_texture = SDL_CreateTexture(window->sdl_renderer, SDL_PIXELFORMAT_YUY2,
            SDL_TEXTUREACCESS_STREAMING, width, height);
    if (window->sdl_texture == NULL) {
        LOGE(NO_DUMP_ERRNO, "%s", SDL_GetError());
        goto free_sdl_renderer;
    }

    return window;

free_sdl_renderer:
    SDL_DestroyRenderer(window->sdl_renderer);
free_sdl_window:
    SDL_DestroyWindow(window->sdl_window);
free_window:
    free(window);
err_return:
    return NULL;
}

void window_update_frame(struct window *window, void *addr, int size) {
    void *pixels;
    static int i = 0;
    int pitch;
    SDL_Event event;
    if (!window) {
        LOGE(NO_DUMP_ERRNO, "Invaild window\n");
        return;
    }
    if (!addr || size <= 0) {
        LOGE(NO_DUMP_ERRNO, "Invaild address or size\n");
        return;
    }
    if (SDL_LockTexture(window->sdl_texture, NULL, &pixels, &pitch)) {
        LOGE(NO_DUMP_ERRNO, "%s", SDL_GetError());
        return;
    }
    memcpy(pixels, addr, size);
    SDL_UnlockTexture(window->sdl_texture);
    if (SDL_RenderCopy(window->sdl_renderer, window->sdl_texture, NULL, NULL)) {
        LOGE(NO_DUMP_ERRNO, "%s", SDL_GetError());
        return;
    }
    SDL_RenderPresent(window->sdl_renderer);

    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_QUIT) {
            exit(0);
        }
    }
}

void window_destory(struct window *window) {
    SDL_DestroyTexture(window->sdl_texture);
    SDL_DestroyRenderer(window->sdl_renderer);
    SDL_DestroyWindow(window->sdl_window);
    SDL_Quit();
    free(window);
}
