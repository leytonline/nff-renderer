#include "ControllerState.h"

using namespace ControllerState;

MovementState::MovementState() {
    _state = 0;
}

MovementState::~MovementState() {}

void MovementState::Reset() {
    _state = 0;
}

void MovementState::HandleInput(SDL_Keycode input, bool isDown) {

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
        case SDLK_SPACE:
            toApply = UP;
            break;
        case SDLK_LCTRL:
            toApply = DOWN;
            break;
        case SDLK_RIGHT:
            toApply = Y_RIGHT;
            break;
        case SDLK_LEFT:
            toApply = Y_LEFT;
            break;
        case SDLK_UP:
            toApply = P_UP;
            break;
        case SDLK_DOWN:
            toApply = P_DOWN;
            break;

    }

    ApplyState(toApply, isDown);
}

void MovementState::ApplyState(MovementEnum move, bool isDown) {
    // due to not offsetting (so enum values match), NONE states should never apply
    if (isDown) _state |= (0b1 << move);
    else _state &= ~(0b1 << move);

}

uint16_t MovementState::GetState() {
    return _state;
}

uint16_t MovementState::GetState(MovementEnum move) {
    return _state & (0b1 << move); // intention to on tick just go: if (GetState(enum) > 0) { do work; }
}

void MovementState::LogDebug() {
    std::cout << "FORWARD : " << GetState(FORWARD) << std::endl;
    std::cout << "BACKWARD: " << GetState(BACKWARD) << std::endl;
    std::cout << "LEFT    : " << GetState(LEFT) << std::endl;
    std::cout << "RIGHT   : " << GetState(RIGHT) << std::endl;
    std::cout << "UP      : " << GetState(UP) << std::endl;
    std::cout << "DOWN    : " << GetState(DOWN) << std::endl;
    std::cout << "Y_RIGHT : " << GetState(Y_RIGHT) << std::endl;
    std::cout << "Y_LEFT  : " << GetState(Y_LEFT) << std::endl;
    std::cout << "P_UP    : " << GetState(P_UP) << std::endl;
    std::cout << "P_DOWN  : " << GetState(P_DOWN) << std::endl;
}