#include "Engine.h"

/*
    TO REFACTOR FURTHER LATER
*/

Engine::Engine() {

}

int Engine::MainLoop() {
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
    if (scene.parse("scenes/test.nff") < 0)
    {
        printf("Failed to parse nff image");
        abort();
    }
    //scene.dumpLong();

    Controller c;
    MetalRasterizer r;
    //r.SetAxisDebug(true);
    r.SetNff(&scene);
    c.InitializeView(scene.GetFrom(), scene.GetUp(), scene.GetAt()); // 0,0,0 at (not always ?)

    // END SETUP
    // BEGIN MAIN LOOP

    // necessities
    SDL_Event e;
    bool running = true;
    ControllerState::MovementState ms;

    // tick rate
    std::chrono::time_point prev_time = std::chrono::high_resolution_clock::now();
    double accumulator = 0.;

    // frame rate
    unsigned int frames       = 0; // generic counter
    double       frameSeconds = 0.0;
    unsigned int currFps      = 0;
    uint64_t     perfFreq     = SDL_GetPerformanceFrequency();
    uint64_t     fpsPrev      = SDL_GetPerformanceCounter();
    char         fpsCountBuf[64];

    while (running)  
    {
        // fps
        uint64_t fpsNow   = SDL_GetPerformanceCounter();
        double   delta    = double(fpsNow - fpsPrev) / double(perfFreq);
        fpsPrev           = fpsNow;

        // 2) count frames and accumulate seconds
        frames++;
        frameSeconds += delta;

        // 3) once per second, update FPS
        if (frameSeconds >= 1.0) {
            currFps      = frames;
            frames       = 0;
            frameSeconds -= 1.0;  // subtract one second

            std::snprintf(fpsCountBuf, sizeof(fpsCountBuf), "FPS: %u", currFps);
            SDL_SetWindowTitle(window, fpsCountBuf);
        }

        // ticks
        auto now = std::chrono::high_resolution_clock::now();
        accumulator += std::chrono::duration<double>(now - prev_time).count();
        prev_time = now;
        while (SDL_PollEvent(&e)) 
        {
            if (e.type == SDL_QUIT) running = false;

            if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) { // doesn't include mouse input ?
                ms.HandleInput(e.key.keysym.sym, e.type == SDL_KEYDOWN);
            }
        }

        // tick for however long we need to (in case we hung for a while)
        while (accumulator >= TICK_RATE)
        {
            c.Tick(TICK_RATE, ms);
            accumulator -= TICK_RATE;
        }

        // lerp
        //double alpha = accumulator / TICK_RATE;

        // lerp here
        r.Render(px, c.GetPosition(), c.GetOrientation());
        SDL_UpdateTexture(texture, nullptr, px, WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        SDL_Delay(10); 
    }


    // END MAIN LOOP

    delete[] px;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}