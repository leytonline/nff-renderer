#include "Movement.h"

using namespace Movement;

MovementState::MovementState() {
    _state = 0;
}

MovementState::~MovementState() {}

void MovementState::ApplyState(MovementEnum move) {
    _state |= move;
}

uint8_t MovementState::GetState() {
    return _state;
}