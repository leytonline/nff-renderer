#include "Controller.h"

constexpr double ANGLE = std::numbers::pi_v<double> / 180.;

Controller::Controller() {
    // maybe make this have more functionality? 
}

void Controller::HandleMouse(SDL_MouseMotionEvent mme) {
    double sens = 0.002;

    yaw(-mme.xrel * sens);
    pitch(-mme.yrel * sens);
}

void Controller::Tick(double tr, ControllerState::MovementState state) {

    double multiplier = 1;
    if (state.GetState(ControllerState::MovementEnum::FAST)) {
        multiplier *= 2;
    }
    if (state.GetState(ControllerState::MovementEnum::SLOW)) {
        multiplier *= 0.5;
    }

    if (state.GetState(ControllerState::MovementEnum::FORWARD)) {
        moveForward(multiplier);
    }

    if (state.GetState(ControllerState::MovementEnum::BACKWARD)) {
        moveBackward(multiplier);
    }

    if (state.GetState(ControllerState::MovementEnum::LEFT)) {
        //rotateLeft();
        moveLeft(multiplier);
    }

    if (state.GetState(ControllerState::MovementEnum::RIGHT)) {
        //rotateRight();
        moveRight(multiplier);
    }

    if (state.GetState(ControllerState::MovementEnum::UP)) {
        // rotateUp();
        moveUp(multiplier);
    }

    if (state.GetState(ControllerState::MovementEnum::DOWN)) {
        // rotateDown();
        moveDown(multiplier);
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
    _initialPos = _pos = p; // view point
    _up = up.normalized();
    _initialAt = _at = at;
    

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
    _orientation = (q * _orientation).normalized();

}

void Controller::pitch(double rot) {
    Eigen::Vector3d right = _orientation * Eigen::Vector3d::UnitX();
    Eigen::Quaterniond q(Eigen::AngleAxisd(rot, right));
    _orientation = (q * _orientation).normalized();
}

void Controller::moveForward(double factor) {
    Eigen::Vector3d forward = -(_orientation * Eigen::Vector3d::UnitZ()).normalized();
    _pos += factor * 0.01 * intendedDistance() * forward;
}

void Controller::moveBackward(double factor) {
    Eigen::Vector3d backward = (_orientation * Eigen::Vector3d::UnitZ()).normalized();
    _pos += factor * 0.01 * intendedDistance() * backward;
}

void Controller::moveLeft(double factor) {
    Eigen::Vector3d left = -(_orientation * Eigen::Vector3d::UnitX()).normalized();
    _pos += factor * 0.01 * intendedDistance() * left;
}

void Controller::moveRight(double factor) {
    Eigen::Vector3d right = (_orientation * Eigen::Vector3d::UnitX()).normalized();
    _pos += factor * 0.01 * intendedDistance() * right;
}

void Controller::moveUp(double factor) {
    _pos += factor * 0.01 * intendedDistance() * Eigen::Vector3d::UnitZ();
}

void Controller::moveDown(double factor) {
    _pos += factor * 0.01 * intendedDistance() * -Eigen::Vector3d::UnitZ();
}

// Returns a non-normalized "view" direction
// mostly for helping get axis to rotate up/down on, hence not normalized (so cross works nicely)
// deprecate, not viewDir anymore but our "connection" to world center
Eigen::Vector3d Controller::viewDir() const {
    return _initialAt - _pos;
}

double Controller::intendedDistance() const {
    return (_initialAt - _initialPos).norm();
}

// Gets the axis to rotate up/down around on
// points to the left of camera
Eigen::Vector3d Controller::horizontalAxis() const {
    return _up.cross(viewDir()).normalized();
}

const Eigen::Quaterniond& Controller::GetOrientation() const {
    return _orientation;
}