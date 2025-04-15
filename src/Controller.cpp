#include "Controller.h"

#define Z_AXIS Eigen::Vector3d(0,0,1)
#define RAD_30 30. * M_PI / 180. 

Controller::Controller() {
    // maybe make this have more functionality? 
}

void Controller::Handle(ControllerInput::ArrowKey key) {
    switch (key)
    {
        case ControllerInput::UP_ARROW:
            rotateUp();
            break;
        case ControllerInput::DOWN_ARROW:
            rotateDown();
            break;
        case ControllerInput::RIGHT_ARROW:
            rotateRight();
            break;
        case ControllerInput::LEFT_ARROW:
            rotateLeft();
            break;
    }
}

void Controller::InitializeView(const Eigen::Vector3d& p) {
    _pos = p; // view point

    // have to get the line orthogonal to both at-from AND then project it orthogonal to NFF up
    _axis = Eigen::Vector3d(p[1], -p[0], p[2]).normalized(); // perpendicular to view directed left of view, flattened onto xy plane
}

const Eigen::Vector3d& Controller::GetPosition() {
    return _pos;
}

void Controller::rotateRight() {
    Eigen::AngleAxisd right(RAD_30, Z_AXIS);
    _pos = right * _pos;
    updateAxis(right);
}

void Controller::rotateLeft() {
    Eigen::AngleAxisd left(-RAD_30, Z_AXIS);
    _pos = left * _pos;
    updateAxis(left);
}

void Controller::rotateUp() {
    Eigen::AngleAxisd up(RAD_30, _axis);
    _pos = up  * _pos;
}

void Controller::rotateDown() {
    Eigen::AngleAxisd down(-RAD_30, _axis);
    _pos = down * _pos;
}

void Controller::updateAxis(const Eigen::AngleAxisd& rot) {
    _axis = rot * _axis;
    _axis.normalize();
}


