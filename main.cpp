#include <stdio.h>                 
#include <ftxui/dom/elements.hpp> 
#include <ftxui/screen/screen.hpp> 
#include <vector>                   
#include "ftxui/dom/canvas.hpp"  
#include "ftxui/dom/node.hpp" 
#include "ftxui/screen/color.hpp" 
#include "Rasterizer.h"

using namespace ftxui;

int main() {
    auto c = Canvas(512, 512);
    Rasterizer r("scenes/teapot-3.nff", &c);
    r.pipeline();
    auto document = canvas(&c) | border;
    auto screen = Screen::Create(Dimension::Fit(document));
    Render(screen, document);
    screen.Print();
    getchar();

    return 0;
}