#include <cstdint>
#include <SDL2/SDL.h>    

#ifndef MOVEMENTSTATE_H
#define MOVEMENTSTATE_H

namespace Movement {

    enum MovementEnum{NONE, FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN};

    class MovementState {
        public:
            MovementState();
            ~MovementState(); // unneeded 
            void Reset();
            uint8_t GetState();
            uint8_t GetState(MovementEnum);
            void HandleInput(SDL_Keycode);
            void ApplyState(MovementEnum);
        private:
            uint8_t _state; // byte
    };
}

#endif // MOVEMENTSTATE_H