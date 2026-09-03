#include "NaiveRaytracer.h"

// constructor
NaiveRaytracer::NaiveRaytracer() {
    _nff = nullptr;
    _samples = 1;
    _jitter = false;
    _phong = false;
    _dof = false;
    _apSize = 0;
}

void NaiveRaytracer::SetNff(Nff* n) {
    _nff = n;
    _bvh = BVH(n->_surfaces);
}

// Xi value for jittering (textbook called it Xi in ch. 13)
inline double getRand() { return (double) rand() / RAND_MAX;}

// the main event of ray tracing
void NaiveRaytracer::Render(uint32_t* out, const Eigen::Vector3d& pos, const Eigen::Quaterniond& orientation) {

    // could also do -(_at - _from)
    double dist = (_nff->_from - _nff->_at).norm();

    // height or size of pixel
    double h = tan((M_PI/180.0) * (_nff->_angle/2.0)) * dist;
    double increment = (2*h) / _nff->_res.first;
    double l = -h + 0.5*increment;
    double t = h*(((double)_nff->_res.second)/_nff->_res.first) - 0.5*increment;
    uint8_t red = 0, green = 0, blue = 0;

    Eigen::Vector3d forward = (orientation * -Eigen::Vector3d::UnitZ()).normalized();
    Eigen::Vector3d right = (orientation * Eigen::Vector3d::UnitX()).normalized();
    Eigen::Vector3d up = (orientation * Eigen::Vector3d::UnitY()).normalized();

    #pragma omp parallel for
    for (int j = 0; j < _nff->_res.second; j++)
    {
        // calculate b(j) since going scan line order
        for (int i = 0; i < _nff->_res.first; i++)
        {
            // assume the pixel is going to have the background color
            Eigen::Vector3d px = Eigen::Vector3d::Zero();

            for (int p = 0; p < (_jitter ? _samples : 1); p++)
            {
                for (int q = 0; q < (_jitter ? _samples : 1); q++)
                {
                    double pJit = 0, qJit = 0;

                    // if jittering pixels, calculate that jitter value
                    if (_jitter)
                    {
                        pJit = (p+getRand()) / _samples;
                        qJit = (q+getRand()) / _samples;
                    }
                    
                    // this will ultimately be the primary ray
                    double a = l + (i + pJit)*increment;
                    double b = t - (j + qJit)*increment;
                    // the point we are looking at
                    Eigen::Vector3d pt = a * right + b * up + dist * forward + pos;

                    // ray we are projecting
                    Ray r((pt - pos).normalized(), pos);

                    double randX = 0, randY = 0;

                    if (_dof)
                    {
                        // generate a random number within a disc with radius of provided aperture size
                        for (int k = 0; k < _samples*_samples; k++)
                        {
                            double rand = _apSize * sqrt(getRand());
                            randX = rand * cos(getRand() * 2 * M_PI);
                            randY = rand * sin(getRand() * 2 * M_PI);
                            Eigen::Vector3d dofEye = pos + randX * right + randY * up;
                            Eigen::Vector3d dofDir = pt - dofEye;
                            dofDir.normalize();
                            Eigen::Vector3d converg = dofEye + ((pos + dist * forward) - dofEye).norm() * dofDir;

                            Ray dofR((converg - dofEye).normalized(), dofEye);

                            // if we're jittering, we have to divide by _samples^4, otherwise dof alone just samples^2
                            px += castRay(dofR, 0, INFINITY) / ((_jitter ? _samples*_samples*_samples*_samples : _samples*_samples));
                        }
                    }

                    // with no dof, calculate what ill will normally be
                    if (!_dof) px += castRay(r, 0, INFINITY) / (_samples*_samples);

                    red = uint8_t(std::min(1.0, std::max(0.0, px[0])) * 255.0);
                    green = uint8_t(std::min(1.0, std::max(0.0, px[1])) * 255.0);
                    blue = uint8_t(std::min(1.0, std::max(0.0, px[2])) * 255.0);

                    // manual index due to paralellization
                    out[j * _nff->_res.first + i] = (red << 16) | (green << 8) | blue;
                }
            }
        }
    }
}

Eigen::Vector3d NaiveRaytracer::castRay(Ray& r, double t0, double t1) {
    HitRecord hr;
    Eigen::Vector3d ret(_nff->_bg);

    if (r.getDepth() > MAX_BOUNCES) return ret;

    bool hit = false;

    hit = _bvh.intersect(r, t0, t1, hr);

    if (hit) {
        hr._rayDepth = r.getDepth();
        hr._v = (r.getOrigin() - hr._p).normalized();
        ret = shade(hr);
    }

    return ret;
}

Eigen::Vector3d NaiveRaytracer::shade(HitRecord& hr) {

    Eigen::Vector3d ret(0,0,0);

    //return hr._fill._color;

    double intensity = 1 / sqrt(_nff->_lights.size());

    Fill& f = hr._fill;
    
    for (size_t i = 0; i < _nff->_lights.size(); i++)
    {
        Light& light = _nff->_lights[i];
        Eigen::Vector3d lightDir = light._coords - hr._p;
        lightDir.normalize();
        Ray r(lightDir, hr._p);

        bool shadow = _bvh.intersectAny(r, 1e-6, (light._coords - hr._p).norm());    

        if (!shadow)
        {
            Eigen::Vector3d half = (lightDir + hr._v) / (lightDir + hr._v).norm();
            half.normalize();
            double diffuse = std::fmax(0, hr._n.dot(lightDir));
            double specular = pow(std::fmax(0, hr._n.dot(half)), f._shine);
            ret += (f._color * diffuse * f._kd + f._ks*Eigen::Vector3d(specular,specular,specular)) * intensity;
        }

    }

    // max bounces + 1 here so that the 6th call (first castRAy with depth > 5, it will just let the castRay return BG color)
    if (hr._rayDepth <= MAX_BOUNCES + 1)
    {
        Eigen::Vector3d dir = -hr._v;
        dir.normalize();

        Eigen::Vector3d normal = hr._n;
        normal.normalize(); // prob should normalize on hit... but after refactor not going to verify
        

        Eigen::Vector3d reflectionDir = dir - 2.0 * dir.dot(normal) * normal;
        reflectionDir.normalize();

        // foundations of comp graphics ch 13.1
        if (f._transmittance > 0.)
        {
            bool isEntering = dir.dot(normal) < 0.;

            double incidence, transmit;
            Eigen::Vector3d refractNormal;

            if (isEntering)
            {
                incidence = 1.0;
                transmit = f._index;
                refractNormal = normal;
            }
            else
            {
                incidence = f._index;
                transmit = 1.0;
                refractNormal = -normal;
            }

            double refractRatio = incidence / transmit;
            double cosAngle = std::min(1.0, -dir.dot(refractNormal));
            
            Eigen::Vector3d refractOrthog = refractRatio * (dir + cosAngle * refractNormal);

            double psquared = 1.0 - refractOrthog.squaredNorm();

            double r0 = (incidence - transmit) / (incidence + transmit);
            r0 *= r0;

            double fresnel = r0 + (1.0 - r0) * std::pow(1.0 - cosAngle, 5.);

            Ray reflectionRay(reflectionDir, hr._p);
            reflectionRay.setDepth(hr._rayDepth + 1);

            Eigen::Vector3d reflectionColor = castRay(reflectionRay, 1e-6, std::numeric_limits<double>::infinity());

            Eigen::Vector3d dielectric = Eigen::Vector3d::Zero();

            // total internal reflection test
            if (psquared < 0.)
            {
                dielectric = reflectionColor;
            }
            else
            {
                // actual refraction calculation
                Eigen::Vector3d refractDir = refractOrthog - std::sqrt(psquared) * refractNormal;
                refractDir.normalize();

                Ray refractRay(refractDir, hr._p);
                refractRay.setDepth(hr._rayDepth + 1);

                Eigen::Vector3d refractColor = castRay(refractRay, 1e-6, std::numeric_limits<double>::infinity());

                Eigen::Vector3d transmitColor = f._color.cwiseProduct(refractColor);

                dielectric = fresnel * reflectionColor + (1. - fresnel) * transmitColor;
            }

            ret += f._transmittance * dielectric;
        }
        else if (f._ks > 0.) // no transmittance color
        {
            Ray reflectionRay(reflectionDir, hr._p);
            reflectionRay.setDepth(hr._rayDepth + 1);

            ret += f._ks * castRay(reflectionRay, 1e-6, std::numeric_limits<double>::infinity());

        }
    }

    return ret;
    
}
