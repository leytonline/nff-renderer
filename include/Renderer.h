#include "Nff.h"

#ifndef RENDERER_H
#define RENDERER_H

class Renderer {
public:
    Renderer();
    virtual ~Renderer();
    virtual void Render(uint32_t*, const Eigen::Vector3d&) = 0;
protected:
    Nff* _nff;
};

#endif