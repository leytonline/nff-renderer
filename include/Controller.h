#include <Eigen/Dense>
#include <SDL2/SDL.h> 
#include <numbers>
#include "ControllerState.h"   

#ifndef CONTROLLER_H
#define CONTROLLER_H

class Controller {
public:
    Controller();
    ~Controller() {} // trivial

    // handle input from the user, calculate update new camera pos and adjust rotation axis
    void HandleMouse(SDL_MouseMotionEvent);
    
    void Tick(double, ControllerState::MovementState);

    // load the initial view point from the scene, define up/down rotation axis as direction pointing left of camera
    void InitializeView(const Eigen::Vector3d&, const Eigen::Vector3d&, const Eigen::Vector3d&);

    // Get the position the view is currently at (for the point to render from)
    const Eigen::Vector3d& GetPosition();

    // get the viewing dir
    const Eigen::Quaterniond& GetOrientation() const;
private:
    void rotateRight();
    void rotateLeft();
    void rotateUp();
    void rotateDown();
    void yaw(double);
    void pitch(double);
    void moveForward(double factor);
    void moveBackward(double factor);
    void moveLeft(double factor);
    void moveRight(double factor);
    void moveUp(double factor);
    void moveDown(double factor);
    Eigen::Vector3d viewDir() const;
    Eigen::Vector3d horizontalAxis() const;
    double intendedDistance() const;
    Eigen::Vector3d _pos, _up, _at, _initialAt, _initialPos; // world 
    Eigen::Quaterniond _orientation;
};

#endif