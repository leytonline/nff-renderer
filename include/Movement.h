#include <cstdint>

#ifndef MOVEMENTSTATE_H
#define MOVEMENTSTATE_H

namespace Movement {

    enum MovementEnum{FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN};

    class MovementState {
        public:
            MovementState();
            ~MovementState(); // unneeded
            uint8_t GetState();
            void ApplyState(MovementEnum);
        private:
            uint8_t _state; // byte
    };
}

#endif // MOVEMENTSTATE_H