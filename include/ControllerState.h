#include <cstdint>
#include <iostream>
#include <SDL2/SDL.h>    

#ifndef CONTROLLERSTATE_H
#define CONTROLLERSTATE_H

namespace ControllerState {

    enum MovementEnum{NONE, FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN, Y_LEFT, Y_RIGHT, P_UP, P_DOWN};

    // binary movements and looking (arrowkey yaw/pitch)
    class MovementState {
        public:
            MovementState();
            ~MovementState(); // unneeded 
            void Reset();
            uint16_t GetState();
            uint16_t GetState(MovementEnum);
            void HandleInput(SDL_Keycode, bool);
            void ApplyState(MovementEnum, bool);
            void LogDebug();
        private:
            uint16_t _state; // word
    };

}

#endif // MOVEMENTSTATE_H