#include "NaiveRasterizer.h"
#include <cmath>

NaiveRasterizer::NaiveRasterizer() {
    _transparent = false;
    _fragmentShading = false;
    _debug = false;
}

// the graphics pipeline
void NaiveRasterizer::Render(uint32_t* pixels, const Eigen::Vector3d& pos, const Eigen::Quaterniond& dir) {

    Eigen::Matrix4d m = calcM(pos, dir);

    // vertex processing -> transform geometry and shade vertices
    std::vector<Triangle> transformedGeos;
    processVertices(transformedGeos, m);

    // rasterize -> generate fragments based on transformed geometry
    std::vector<Fragment>* fragments = new std::vector<Fragment>[_nff->_res.first * _nff->_res.second]();
    rasterize(transformedGeos, fragments);

    if (_fragmentShading) processFragments(fragments);

    // blending
    Eigen::Vector3d* image = new Eigen::Vector3d[_nff->_res.first * _nff->_res.second]();
    blend(fragments, image);

    //if (_debug) drawDebugAxes(image, pos);

    writeImage(image, pixels);

    delete[] fragments;
    delete[] image;
}

// get the entire projection matrix M
Eigen::Matrix4d NaiveRasterizer::calcM(const Eigen::Vector3d& pos, const Eigen::Quaterniond& dir) {

    Eigen::Vector3d forward = dir * -Eigen::Vector3d::UnitZ();  
    Eigen::Vector3d right   = dir * Eigen::Vector3d::UnitX();   
    Eigen::Vector3d up      = dir * Eigen::Vector3d::UnitY();   
    Eigen::Matrix3d view;
    view << right, up, -forward;

    // calculate all of the stuff needed
    double near = -_nff->_hither;
    double far = 1000 * near;
    double h = tan(_nff->_angle / 2 * M_PI / 180);
    double l = -1 * h, r = 1 * h, b = -1 * h, t = 1 * h;

    // all calculated as the textbook specified
    Eigen::Matrix4d Mvp, P, Morth;
    P << near, 0, 0, 0, 
         0, near, 0, 0, 
         0, 0, near + far, -(far*near), 
         0, 0, 1, 0;
    Mvp << _nff->_res.first / 2, 0, 0, (double) (_nff->_res.first - 1) / 2, 
           0, -_nff->_res.second / 2, 0, (double) (_nff->_res.second - 1) / 2,
           0, 0, 1, 0,
           0, 0, 0, 1;
    Morth << 2 / (r - l), 0, 0, 0,
             0, 2 / (t - b), 0, 0,
             0, 0, 2 / (near - far), - (near + far) / (near - far),
             0, 0, 0, 1;
    Eigen::Matrix4d Mcam = Eigen::Matrix4d::Identity();
    Mcam.block<3,3>(0,0) = view.transpose();            // rotate world into camera space
    Mcam.block<3,1>(0,3) = -view.transpose() * pos;   

    return Mvp * Morth * P * Mcam;
}

// transform and shade vertices, placing them in transforms vector
void NaiveRasterizer::processVertices(std::vector<Triangle>& transforms, Eigen::Matrix4d m) {

    for (unsigned i = 0; i < _nff->_surfaces.size(); i++)
    {
        Triangle& tri = *_nff->_surfaces[i];
        std::vector<Eigen::Vector3d> verts;
        std::vector<double> divs;

        for (int j = 0; j < 3; j++)
        {
            Eigen::Vector4d homogenized;
            homogenized << tri._vertices[j], 1;
            Eigen::Vector4d result = m * homogenized;
            verts.push_back(Eigen::Vector3d(result[0], result[1], result[2]));
            divs.push_back(result[3]);
        }

        // create the new triangle with the transformed points and some default fill since it won't get used.
        Triangle transformed(verts, tri._fill);

        if (tri._patch) 
        {
            transformed._patch = true;
            transformed._origNorms = static_cast<Tripatch*>(&tri)->_norms;
        };

        // might needs these for fragment shading
        transformed._origVerts = tri._vertices;

        // store the homogenous divide parallel to it's vertex in _divs
        transformed._divs = divs;

        // apply vertex shading
        shadeTriangle(&tri, transformed._shades);
        transforms.push_back(transformed);
    }

}

// given some vertex, shade it. Assumes world coordinates for V
// wil put the shaded triangle in the referenced vector
void NaiveRasterizer::shadeTriangle(Triangle* t, std::vector<Eigen::Vector3d>& c) {

    double intensity = 1 / sqrt(_nff->_lights.size());
    Fill& f = t->_fill;

    Eigen::Vector3d normal(0,0,0);

    // if this was a fanned triangle, we need to use the v0,v1,v2 normal
    normal = t->_normalOverride ? t->_normal : (t->_vertices[0] - t->_vertices[1]).cross(t->_vertices[0] - t->_vertices[2]);
    normal.normalize();

    // at each vertex calculate the shading
    for (unsigned i = 0; i < 3; i++)
    {
        Eigen::Vector3d& vert = t->_vertices[i];
        Eigen::Vector3d color(0,0,0);
        
        // if it is a patch, get the normal for that vertex
        if (t->_patch)
        {
            normal = static_cast<Tripatch*>(t)->_norms[i];
            normal.normalize();
            // possibly used later in fragment shading
        }

        // do diffuse and specular calculations per each light
        for (unsigned j = 0; j < _nff->_lights.size(); j++)
        {
            Light& light = _nff->_lights[j];
            Eigen::Vector3d viewDir = _nff->_from - vert;
            Eigen::Vector3d lightDir = light._coords - vert;
            viewDir.normalize(); 
            lightDir.normalize();
            Eigen::Vector3d half = (lightDir + viewDir) / (lightDir + viewDir).norm();
            half.normalize();
            double diffuse = std::fmax(0, normal.dot(lightDir));
            double specular = pow(std::fmax(0, normal.dot(half)), f._shine);
            color += (f._color * diffuse * f._kd + f._ks * Eigen::Vector3d(specular,specular,specular)) * intensity;
        }

        // associate the color with the vertex (parallel vectors)
        c.push_back(color);
    }

}

// calculate bounding boxes and generate fragments
void NaiveRasterizer::rasterize(std::vector<Triangle>& tris, std::vector<Fragment>* frags) {

    for (unsigned i = 0; i < tris.size(); i++)
    {
        // do this per triangle just to avoid a ton of nesting
        raster(tris[i], frags);
    }
}

// these are for alpha beta gamma just from the textbook
inline double f01(double x, double y, Triangle& t) {
    return (t._vertices[0][1] - t._vertices[1][1]) * x 
           + (t._vertices[1][0] - t._vertices[0][0]) * y 
           + t._vertices[0][0] * t._vertices[1][1]
           - t._vertices[1][0] * t._vertices[0][1];
}

inline double f12(double x, double y, Triangle& t) {
    return (t._vertices[1][1] - t._vertices[2][1]) * x 
           + (t._vertices[2][0] - t._vertices[1][0]) * y 
           + t._vertices[1][0] * t._vertices[2][1]
           - t._vertices[2][0] * t._vertices[1][1];
}

inline double f20(double x, double y, Triangle& t) {
    return (t._vertices[2][1] - t._vertices[0][1]) * x 
           + (t._vertices[0][0] - t._vertices[2][0]) * y 
           + t._vertices[2][0] * t._vertices[0][1]
           - t._vertices[0][0] * t._vertices[2][1];
}

void NaiveRasterizer::raster(Triangle& t, std::vector<Fragment>* frags) {

    int bxMin = _nff->_res.first, bxMax = 0, byMin = _nff->_res.second, byMax = 0;

    // homogenous divide
    for (int i = 0; i < 3; i++)
    {
        // divide each x,y leaving it Z unchanged
        t._vertices[i][0] /= t._divs[i];
        t._vertices[i][1] /= t._divs[i];
    }

    // calculate bounding box
    for (int i = 0; i < 3; i++)
    {
        bxMin = std::min((int) floor(t._vertices[i][0]), bxMin);
        bxMax = std::max((int) ceil(t._vertices[i][0]), bxMax);
        byMin = std::min((int) floor(t._vertices[i][1]), byMin);
        byMax = std::max((int) ceil(t._vertices[i][1]), byMax);
    }

    // clamp to image resolution
    bxMin = std::max(0, bxMin);
    bxMax = std::min(_nff->_res.first, bxMax);
    byMin = std::max(0, byMin);
    byMax = std::min(_nff->_res.first, byMax);

    double fa = f12(t._vertices[0][0], t._vertices[0][1], t);
    double fb = f20(t._vertices[1][0], t._vertices[1][1], t);
    double fg = f01(t._vertices[2][0], t._vertices[2][1], t);

    for (int j = byMin; j < byMax; j++)
    {
        for (int i = bxMin; i < bxMax; i++)
        {
            // calculate
            double alpha = 0, beta = 0, gamma = 0;
            alpha = f12(i, j, t) / fa;
            beta = f20(i, j, t) / fb;
            gamma = f01(i, j, t) / fg;

            // check if point in triangle
            if (alpha >= 0 && beta >= 0 && gamma >= 0)
            {
                if ((alpha > 0 || fa * f12(-1, -1, t) > 0) 
                    && (beta > 0 || fb * f20(-1, -1, t) > 0) 
                    && (gamma > 0 || fg * f01(-1, -1, t) > 0))
                {
                    // calculate the color
                    Eigen::Vector3d color = t._shades[0] + beta * (t._shades[1] - t._shades[0]) + gamma * (t._shades[2] - t._shades[0]);

                    // interpolate z value
                    double z = alpha * t._vertices[0][2] + beta * t._vertices[1][2] + gamma * t._vertices[2][2];

                    if (z < 0) { 
                        //std::cout << "negative Z: " << z << std::endl;
                        continue;
                    }

                    // add fragment to image vector
                    frags[j * _nff->_res.second + i].push_back(Fragment(color, z));

                    // add attributes to fragments if fragment processing
                    if (_fragmentShading && t._patch)
                    {
                        Tripatch* tri = static_cast<Tripatch*>(&t);
                        frags[j * _nff->_res.second + i].back()._attrNormal = tri->_origNorms[0] + 
                            beta * (tri->_origNorms[1] - tri->_origNorms[0]) + 
                            gamma * (tri->_origNorms[2] - tri->_origNorms[0]);
                        frags[j * _nff->_res.second + i].back()._attrNormal.normalize();
                        frags[j * _nff->_res.second + i].back()._attrFill = t._fill;
                        frags[j * _nff->_res.second + i].back()._isFragShaded = true;
                        frags[j * _nff->_res.second + i].back()._attrPos = tri->_origVerts[0] + 
                            beta * (tri->_origVerts[1] - tri->_origVerts[0]) + 
                            gamma * (tri->_origVerts[2] - tri->_origVerts[0]);
                    }
                }
            }
        }
    }
}

// sort fragments
struct cmpr {
    bool operator() (Fragment& f1, Fragment& f2) { return f1._z < f2._z; }
};

// blend the fragments into the image
void NaiveRasterizer::blend(std::vector<Fragment>* frags, Eigen::Vector3d* im) {
    for (int i = 0; i < _nff->_res.first * _nff->_res.second; i++, im++)
    {
        // assume background color
        *im = _nff->_bg;

        // if theres no fragments just continue to next iteration
        if (frags[i].empty()) continue;

        std::sort(frags[i].begin(), frags[i].end(), cmpr());

        // continue to avoid loop inside an if statement
        if (!_transparent) 
        {
            *im = frags[i].begin()->_color;
            continue;
        }

        //*im = Eigen::Vector3d(0,0,0);
        for (std::vector<Fragment>::reverse_iterator fItr = frags[i].rbegin(); fItr < frags[i].rend(); fItr++)
        {
            *im = ((_transparency * fItr->_color) + ((1.0 - _transparency) * *im));
        }
    }
}

void NaiveRasterizer::writeImage(Eigen::Vector3d* im, uint32_t* pixels) {
    // outputting the image
    int r, g, b;
    for (int y=0; y<_nff->_res.second; y++) {
        for (int x=0; x<_nff->_res.first; x++, im++, pixels++) {
            r = uint8_t(std::min(1.0, std::max(0.0, (*im)[0])) * 255.0);
            g = uint8_t(std::min(1.0, std::max(0.0, (*im)[1])) * 255.0);
            b = uint8_t(std::min(1.0, std::max(0.0, (*im)[2])) * 255.0);
            *pixels = (r << 16) | (g << 8) | b;
        }
    }
}

void NaiveRasterizer::processFragments(std::vector<Fragment>* f) {

    double intensity = 1 / sqrt(_nff->_lights.size());

    for (int i = 0; i < _nff->_res.first * _nff->_res.second; i++)
    {
        for (size_t j = 0; j < f[i].size(); j++)
        {
            Fragment& frag = f[i][j];
            if (!frag._isFragShaded) continue;
            Eigen::Vector3d color(0,0,0);
            // do diffuse and specular calculations per each light
            for (unsigned k = 0; k < _nff->_lights.size(); k++)
            {
                Light& light = _nff->_lights[k];
                Eigen::Vector3d viewDir = _nff->_from - frag._attrPos;
                Eigen::Vector3d lightDir = light._coords - frag._attrPos;
                viewDir.normalize(); 
                lightDir.normalize();
                Eigen::Vector3d half = (lightDir + viewDir) / (lightDir + viewDir).norm();
                half.normalize();
                double diffuse = std::fmax(0, frag._attrNormal.dot(lightDir));
                double specular = pow(std::fmax(0, frag._attrNormal.dot(half)), frag._attrFill._shine);
                color += (frag._attrFill._color * diffuse * frag._attrFill._kd + frag._attrFill._ks * Eigen::Vector3d(specular,specular,specular)) * intensity;
            }
            f[i][j]._color = color;
        }
    }
}

void NaiveRasterizer::SetNff(Nff* n) {
    _nff = n;
}

// void NaiveRasterizer::SetAxisDebug(bool b) {
//     _debug = b;
// }

// // yeah we gotta reevaluate this later, note 2: its ok for now, maybe performance degradation?
// void NaiveRasterizer::drawDebugAxes(Eigen::Vector3d* im, const Eigen::Vector3d& pos) {
//     Eigen::Matrix4d m = calcM(pos);
//     double scale = 5.0;
//     Eigen::Vector3d origin(0, 0, 0);
//     Eigen::Vector3d x(1, 0, 0), y(0, 1, 0), z(0, 0, 1), up = _nff->_up.normalized();

//     auto project = [&](const Eigen::Vector3d& v) {
//         Eigen::Vector4d h; h << v, 1;
//         Eigen::Vector4d r = m * h;
//         r /= r[3];
//         return Eigen::Vector2i((int) round(r[0]), (int) round(r[1]));
//     };

//     auto draw = [&](Eigen::Vector2i from, Eigen::Vector2i to, Eigen::Vector3d color) {
//         int x0 = from[0], y0 = from[1], x1 = to[0], y1 = to[1];
//         int dx = abs(x1 - x0), dy = abs(y1 - y0);
//         int sx = x0 < x1 ? 1 : -1;
//         int sy = y0 < y1 ? 1 : -1;
//         int err = dx - dy;

//         while (true) {
//             if (x0 >= 0 && x0 < _nff->_res.first && y0 >= 0 && y0 < _nff->_res.second)
//                 im[y0 * _nff->_res.first + x0] = color;
//             if (x0 == x1 && y0 == y1) break;
//             int e2 = 2 * err;
//             if (e2 > -dy) { err -= dy; x0 += sx; }
//             if (e2 < dx) { err += dx; y0 += sy; }
//         }
//     };

//     Eigen::Vector2i o = project(origin);
//     draw(o, project(origin + scale * x), Eigen::Vector3d(1, 0, 0)); // x axis red
//     draw(o, project(origin + scale * y), Eigen::Vector3d(0, 1, 0)); // y axis green
//     draw(o, project(origin + scale * z), Eigen::Vector3d(0, 0, 1)); // z axis blue
//     draw(o, project(origin + scale * up), Eigen::Vector3d(1, 1, 0)); // up yellow
// }