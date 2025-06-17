#include "Engine.h"

int main() {

    Engine e;
    if (e.MainLoop() != 0)
    {
        std::cout << "ended badly" << std::endl;
    }
    else
    {
        std::cout << "ended gracefully" << std::endl;
    }

   
}