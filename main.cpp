#include <stdio.h>                 
#include <ftxui/dom/elements.hpp> 
#include <ftxui/screen/screen.hpp> 
#include <vector>                   
#include "ftxui/dom/canvas.hpp"  
#include "ftxui/dom/node.hpp" 
#include "ftxui/screen/color.hpp" 
#include "ftxui/component/component.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include "Rasterizer.h"
#include <fstream>
#include <ctime>

using namespace ftxui;

int main() {

    // timing
    std::fstream file("logs.txt", 0x01); // append
    clock_t start, stop;


    auto c = Canvas(512, 512);
    Rasterizer r("scenes/teapot-3.nff");

    auto document = canvas(&c) | border;

    auto renderer = Renderer([&] {
        start = clock();
        r.pipeline(&c);
        stop = clock();
        std::stringstream ss;
        ss << stop-start << '\n';
        file << ss.str();
        return canvas(c);
    });

    auto si = ScreenInteractive::FullscreenPrimaryScreen();

    auto exit = CatchEvent([&] (const Event& evt) {
            if (evt == Event::Return) {
                si.Exit();
            }
            return true;
    });


    si.Loop(renderer | exit);

    file.close();

    return 0;
}