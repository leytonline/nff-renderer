#include <Eigen/Dense>
#include <SDL2/SDL.h>    

#ifndef CONTROLLER_H
#define CONTROLLER_H

class Controller {
public:
    Controller();
    ~Controller() {} // trivial

    // handle input from the user, calculate update new camera pos and adjust rotation axis
    void Handle(SDL_Keycode);

    // load the initial view point from the scene, define up/down rotation axis as direction pointing left of camera
    void InitializeView(const Eigen::Vector3d&, const Eigen::Vector3d&, const Eigen::Vector3d&);

    // Get the position the view is currently at (for the point to render from)
    const Eigen::Vector3d& GetPosition();
private:
    void rotateRight();
    void rotateLeft();
    void rotateUp();
    void rotateDown();
    Eigen::Vector3d viewDir() const;
    Eigen::Vector3d horizontalAxis() const;

    Eigen::Vector3d _pos, _up, _at;
};

#endif