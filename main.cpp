#include <stdio.h>                 
#include <vector>        
#include <SDL2/SDL.h>    
#include "Rasterizer.h"
#include "Controller.h"

#define HEIGHT 512
#define WIDTH 512

int main() {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("graphics",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT);

    uint32_t *px = new uint32_t[WIDTH * HEIGHT];    

    Nff scene;
    if (scene.parse("scenes/teapot-3.nff") < 0)
    {
        printf("Failed to parse nff image");
        abort();
    }

    Controller c;
    Rasterizer r;
    r.SetNff(&scene);
    c.InitializeView(scene.getFrom());

    // Simple wait until window is closed
    SDL_Event e;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        c.Handle(ControllerInput::UP_ARROW);
                        break;
                    case SDLK_DOWN:
                        c.Handle(ControllerInput::DOWN_ARROW);
                        break;
                    case SDLK_LEFT:
                        c.Handle(ControllerInput::LEFT_ARROW);
                        break;
                    case SDLK_RIGHT:
                    c.Handle(ControllerInput::RIGHT_ARROW);
                        break;
                }
            }
        }

        r.pipeline(px, c.GetPosition());
        SDL_UpdateTexture(texture, nullptr, px, WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(10); // chill loop
    }

    delete[] px;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

    return 0;
}