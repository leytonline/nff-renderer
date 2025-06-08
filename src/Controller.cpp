#include "Controller.h"

#define ANGLERAD(theta) theta * M_PI / 180. 
#define ANGLE ANGLERAD(15)

Controller::Controller() {
    // maybe make this have more functionality? 
}

void Controller::Handle(SDL_Keycode input) {
    switch (input)
    {
        case SDLK_w:
            rotateUp();
            break;
        case SDLK_s:
            rotateDown();
            break;
        case SDLK_d:
            rotateRight();
            break;
        case SDLK_a:
            rotateLeft();
            break;
    }
}

void Controller::InitializeView(const Eigen::Vector3d& p, const Eigen::Vector3d& up, const Eigen::Vector3d& at) {
    _pos = p; // view point
    _up = up.normalized();
    _at = at;
}

const Eigen::Vector3d& Controller::GetPosition() {
    return _pos;
}

void Controller::rotateRight() {
    Eigen::AngleAxisd right(ANGLE, _up);
    _pos = right * _pos;
}

void Controller::rotateLeft() {
    Eigen::AngleAxisd left(-ANGLE, _up);
    _pos = left * _pos;
}

void Controller::rotateUp() {
    Eigen::AngleAxisd up(ANGLE, horizontalAxis());
    _pos = up  * _pos;
}

void Controller::rotateDown() {
    Eigen::AngleAxisd down(-ANGLE, horizontalAxis());
    _pos = down * _pos;
}

// Returns a non-normalized view direction
// mostly for helping get axis to rotate up/down on, hence not normalized (so cross works nicely)
Eigen::Vector3d Controller::viewDir() const {
    return _at - _pos;
}

// Gets the axis to rotate up/down around on
Eigen::Vector3d Controller::horizontalAxis() const {
    return _up.cross(viewDir()).normalized();
}