#include "Controller.h"

#define ANGLERAD(theta) theta * M_PI / 180. 
#define ANGLE ANGLERAD(1)

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
        case SDLK_LEFT:
            yaw(ANGLE);
            break;
        case SDLK_RIGHT:
            yaw(-ANGLE);
            break;
    }
}

void Controller::Tick(double tr, ControllerState::MovementState state) {

    if (state.GetState(ControllerState::MovementEnum::FORWARD)) {

    }

    if (state.GetState(ControllerState::MovementEnum::BACKWARD)) {
        
    }

    if (state.GetState(ControllerState::MovementEnum::LEFT)) {
        rotateLeft();
    }

    if (state.GetState(ControllerState::MovementEnum::RIGHT)) {
        rotateRight();
    }

    if (state.GetState(ControllerState::MovementEnum::UP)) {
        rotateUp();
    }

    if (state.GetState(ControllerState::MovementEnum::DOWN)) {
        rotateDown();
    }

    if (state.GetState(ControllerState::MovementEnum::Y_LEFT)) {
        yaw(ANGLE);
    }

    if (state.GetState(ControllerState::MovementEnum::Y_RIGHT)) {
        yaw(-ANGLE);
    }

    if (state.GetState(ControllerState::MovementEnum::P_UP)) {
        pitch(ANGLE);
    }

    if (state.GetState(ControllerState::MovementEnum::P_DOWN)) {
        pitch(-ANGLE);
    }
}

void Controller::InitializeView(const Eigen::Vector3d& p, const Eigen::Vector3d& up, const Eigen::Vector3d& at) {
    _pos = p; // view point
    _up = up.normalized();
    _at = at;

    Eigen::Vector3d initialDir = (_at - _pos).normalized();
    Eigen::Vector3d right = initialDir.cross(_up).normalized();
    Eigen::Vector3d pup = right.cross(initialDir);
    Eigen::Matrix3d init;
    init.col(0) = right;
    init.col(1) = pup;
    init.col(2) = -initialDir;
    _orientation = Eigen::Quaterniond(init);
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

void Controller::yaw(double rot) { 
    Eigen::Quaterniond q(Eigen::AngleAxisd(rot, Eigen::Vector3d::UnitZ()));
    _orientation = q * _orientation;

}

void Controller::pitch(double rot) {
    Eigen::Vector3d right = _orientation * Eigen::Vector3d::UnitX();
    Eigen::Quaterniond q(Eigen::AngleAxisd(rot, right));
    _orientation = q * _orientation;
}

// Returns a non-normalized view direction
// mostly for helping get axis to rotate up/down on, hence not normalized (so cross works nicely)
// deprecate, not viewDir anymore but our "connection" to world center
Eigen::Vector3d Controller::viewDir() const {
    return _at - _pos;
}

// Gets the axis to rotate up/down around on
Eigen::Vector3d Controller::horizontalAxis() const {
    return _up.cross(viewDir()).normalized();
}

const Eigen::Quaterniond& Controller::GetOrientation() const {
    return _orientation;
}