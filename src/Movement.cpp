#include "Movement.h"

using namespace Movement;

MovementState::MovementState() {
    _state = 0;
}

MovementState::~MovementState() {}

void MovementState::Reset() {
    _state = 0;
}

void MovementState::HandleInput(SDL_Keycode input) {

    MovementEnum toApply = NONE;

    switch (input)
    {
        case SDLK_w:
            toApply = FORWARD;
            break;
        case SDLK_s:
            toApply = BACKWARD;
            break;
        case SDLK_d:
            toApply = RIGHT;
            break;
        case SDLK_a:
            toApply = LEFT;
            break;
    }

    ApplyState(toApply);
}

void MovementState::ApplyState(MovementEnum move) {
    // due to not offsetting (so enum values match), NONE states should never apply
    _state |= (0b1 << move);
}

uint8_t MovementState::GetState() {
    return _state;
}

uint8_t MovementState::GetState(MovementEnum move) {
    return _state & move; // intention to on tick just go: if (GetState(enum) > 0) { do work; }
}