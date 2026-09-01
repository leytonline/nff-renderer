#include "Nff.h"
#include "Geometry.h"
#include <algorithm>
#include <cmath>
#include <numeric>

Nff::Nff() {
    clear();
};

Nff::~Nff() {
    for (unsigned i = 0; i < _surfaces.size(); i++)
    {
        delete _surfaces[i];
    }
}

void Nff::clear() {
    _u << 0,0,0;
    _v << 0,0,0;
    _w << 0,0,0;
    _from << 0,0,0;
    _at << 0,0,0;
    _bg << 0,0,0;
    _up << 0,0,0;
    _angle = 0;
    _hither = 0;
    _res = std::make_pair(0, 0);
    _surfaces.clear();
}

void Nff::dump() {
    std::cout << "NFF\n";
    std::cout << "u: " << _u[0] << ' ' << _u[1] << ' ' << _u[2] << '\n';
    std::cout << "v: " << _v[0] << ' ' << _v[1] << ' ' << _v[2] << '\n';
    std::cout << "w: " << _w[0] << ' ' << _w[1] << ' ' << _w[2] << '\n';
    std::cout << "from: " << _from[0] << ' ' << _from[1] << ' ' << _from[2] << '\n';
    std::cout << "at: " << _at[0] << ' ' << _at[1] << ' ' << _at[2] << '\n';
    std::cout << "bg: " << _bg[0] << ' ' << _bg[1] << ' ' << _bg[2] << '\n';
    std::cout << "up: " << _up[0] << ' ' << _up[1] << ' ' << _up[2] << '\n';
    std::cout << "angle: " << _angle << '\n';
    std::cout << "hither: " << _hither << '\n';
    std::cout << "res: " << _res.first << 'x' << _res.second << '\n';
    std::cout << "# surfaces: " << _surfaces.size() << '\n';
}

void Nff::dumpLong(bool truncate) {
    dump();
    unsigned truncSize = 5 > _surfaces.size() ? _surfaces.size() : 5;
    for (unsigned i = 0; i < (truncate ? truncSize : _surfaces.size()); i++) 
    {
        Triangle* triangle = dynamic_cast<Triangle*>(_surfaces[i]);
        Sphere* sphere = dynamic_cast<Sphere*>(_surfaces[i]);

        if (triangle != nullptr) std::cout << *triangle << '\n';
        else if (sphere != nullptr) std::cout << *sphere << '\n';
    }
}

int Nff::parse(std::string f) {

    std::ifstream file(f);

    if (!file.is_open())
    {
        std::cout << "FILE UNABLE TO OPEN" << std::endl;
        return -1;
    }

    char ch;
    std::string line = "", temp = "", skip = "";
    Fill fill;
    Eigen::Vector3d coords = Eigen::Vector3d::Zero();
    std::vector<std::string> failBuffer;
    double radius = 0.;
    int vertices = 0;
    

    while (getline(file, line))
    {
        std::stringstream ss(line);
        ss >> ch;

        switch (ch)
        {
            // background color
            case 'b':
                ss >> _bg[0] >> _bg[1] >> _bg[2];
                break;

            // viewpoint reading
            case 'v':
                //from
                getline(file, temp);
                ss.clear();
                ss.str(temp);
                ss >> skip >> _from[0] >> _from[1] >> _from[2];

                // at
                getline(file, temp);
                ss.clear();
                ss.str(temp);
                ss >> skip >> _at[0] >> _at[1] >> _at[2];
                // up
                getline(file, temp);
                ss.clear();
                ss.str(temp);
                ss >> skip >> _up[0] >> _up[1] >> _up[2];

                // angle
                getline(file, temp);
                ss.clear();
                ss.str(temp);
                ss >> skip >> _angle;

                // hither
                getline(file, temp);
                ss.clear();
                ss.str(temp);
                ss >> skip >> _hither;

                // resolution
                getline(file, temp);
                ss.clear();
                ss.str(temp);
                ss >> skip >> _res.first >> _res.second;
                break;
            
            // save the fill information
            case 'f':
                ss >> fill._color[0] >> fill._color[1] >> fill._color[2] >> fill._kd 
                >> fill._ks >> fill._shine >> fill._transmittance >> fill._index;
                break;

            // polygon saving
            case 'p': {
                bool patch = false;
                Eigen::Vector3d v, n;
                std::vector<Eigen::Vector3d> verts, norms;
                if (line.size() > 1 && line[1] == 'p') 
                {
                    ss >> ch; // eats the second p of pp
                    patch = true;
                }
                ss >> vertices;

                for (int i = 0; i < vertices; i++)
                {
                    getline(file, line);
                    std::stringstream data(line);
                    if (patch) 
                    {
                        data >> v[0] >> v[1] >> v[2] >> n[0] >> n[1] >> n[2];
                        n.normalize();
                        norms.push_back(n);
                    }
                    else
                    {
                        data >> v[0] >> v[1] >> v[2];
                    }
                    verts.push_back(v);
                }

                Eigen::Vector3d normalOverride = Eigen::Vector3d::Zero();
                for (size_t i = 0; i < verts.size(); i++)
                {
                    Eigen::Vector3d& a = verts[i];
                    Eigen::Vector3d& b = verts[(i + 1) % verts.size()];
                    normalOverride[0] += (a[1] - b[1]) * (a[2] + b[2]);
                    normalOverride[1] += (a[2] - b[2]) * (a[0] + b[0]);
                    normalOverride[2] += (a[0] - b[0]) * (a[1] + b[1]);
                }

                std::vector<std::vector<size_t>> clippedVerts = clip(verts);

                for (const std::vector<size_t>& vec : clippedVerts)
                {
                    if (patch)
                    {
                        Tripatch* tri = new Tripatch();

                        tri->addVertex(verts[vec[0]]);
                        tri->addVertex(verts[vec[1]]);
                        tri->addVertex(verts[vec[2]]);

                        tri->addNorm(norms[vec[0]]);
                        tri->addNorm(norms[vec[1]]);
                        tri->addNorm(norms[vec[2]]);
                        tri->_fill = fill;
                        _surfaces.push_back(tri);
                    }
                    else
                    {
                        Triangle* tri = new Triangle();
                        tri->addVertex(verts[vec[0]]);
                        tri->addVertex(verts[vec[1]]);
                        tri->addVertex(verts[vec[2]]);

                        if (vertices > 3 && normalOverride.squaredNorm() > 0.)
                        {
                            tri->_normalOverride = true;
                            tri->_normal = normalOverride;
                        }

                        tri->_fill = fill;
                        _surfaces.push_back(tri);
                    }
                }
                break;
            }
            // sphere parsing
            case 's':
                ss >> coords[0] >> coords[1] >> coords[2] >> radius;
                _surfaces.push_back(new Sphere(coords, radius, fill));
                break;

            case 'l': {
                double x, y, z, r = -1, g = -1, b = -1;
                ss >> x >> y >> z >> r >> g >> b;
                _lights.push_back(Light(x,y,z,r,g,b));
                break;
            }

            // something didn't parse, append it to debug vector
            default:
                failBuffer.push_back(line);
                break;

        }
    }

    calcDir();

    return 0; // successful
}

void Nff::calcDir() {
    _w = -(_at-_from).normalized();
    _u = _up.cross(_w).normalized();
    _v = _u.cross(-_w);
}

int Nff::projectionAxis(const std::vector<Eigen::Vector3d>& verts) {
    Eigen::Vector3d v = Eigen::Vector3d::Zero();

    for (size_t i = 0; i < verts.size(); i++)
    {
        const Eigen::Vector3d& a = verts[i];
        const Eigen::Vector3d& b = verts[(i + 1) % verts.size()];
        v[0] += (a[1] - b[1]) * (a[2] + b[2]);
        v[1] += (a[2] - b[2]) * (a[0] + b[0]);
        v[2] += (a[0] - b[0]) * (a[1] + b[1]);
    }

    if (v.squaredNorm() == 0. && verts.size() >= 3)
    {
        for (size_t i = 0; i + 2 < verts.size(); i++)
        {
            v = (verts[i + 1]-verts[i]).cross(verts[i + 2] - verts[i]);
            if (v.squaredNorm() != 0.) break;
        }
    }

    v = v.cwiseAbs();
    if (v[0] >= v[1] && v[0] >= v[2]) return 0;
    if (v[1] > v[2]) return 1;
    return 2;
}

Eigen::Vector2d Nff::project2d(const Eigen::Vector3d& v, int axis) {
    switch (axis)
    {
        case 0:
            return {v[1], v[2]};
        case 1:
            return {v[0], v[2]};
    }

    return {v[0], v[1]};
}

double Nff::cross2d(const Eigen::Vector2d& a, const Eigen::Vector2d& b, const Eigen::Vector2d& c) {
    Eigen::Vector2d ab = b - a, ac = c - a;
    return ab[0] * ac[1] - ab[1] * ac[0];
}

bool Nff::inTriangle(const Eigen::Vector2d& a, const Eigen::Vector2d& b, const Eigen::Vector2d& c, const Eigen::Vector2d& p) {
    const double eps = 1e-10;
    double ab = cross2d(a, b, p);
    double bc = cross2d(b, c, p);
    double ca = cross2d(c, a, p);
    return (ab > eps && bc > eps && ca > eps) || (ab < -eps && bc < -eps && ca < -eps);
}

std::vector<std::vector<size_t>> Nff::clip(const std::vector<Eigen::Vector3d>& v) {
    const double eps = 1e-10;
    std::vector<std::vector<size_t>> triangles;
    if (v.size() < 3) return triangles;

    if (v.size() == 3)
    {
        triangles.push_back({0, 1, 2});
        return triangles;
    }

    int axis = projectionAxis(v);

    std::vector<Eigen::Vector2d> projected;

    for (const Eigen::Vector3d& vec : v)
    {
        projected.push_back(project2d(vec, axis));
    }

    // projection may screw up stuff
    double area = 0.0;
    for (size_t i = 0; i < projected.size(); i++)
    {
        Eigen::Vector2d& a = projected[i];
        Eigen::Vector2d& b = projected[(i + 1) % projected.size()];
        area += a.x() * b.y() - b.x() * a.y();
    }

    if (std::abs(area) <= eps)
    {
        for (size_t i = 1; i + 1 < v.size(); i++)
        {
            triangles.push_back({0, i, i + 1});
        }
        return triangles;
    }

    double winding = area > 0. ? 1. : -1.;
    std::vector<size_t> remaining(v.size());
    std::iota(remaining.begin(), remaining.end(), 0);

    while (remaining.size() > 3)
    {
        bool clipped = false;
        size_t count = remaining.size();

        for (size_t i = 0; i < count; i++)
        {
            size_t prev = remaining[(i + count - 1) % count];
            size_t curr = remaining[i];
            size_t next = remaining[(i + 1) % count];

            Eigen::Vector2d& a = projected[prev], &b = projected[curr], &c = projected[next];

            if (winding * cross2d(a, b, c) <= eps) continue;

            bool contained = false;

            for (size_t idx : remaining)
            {
                if (idx == prev || idx == curr || idx == next) continue;

                if (inTriangle(a,b,c, projected[idx]))
                {
                    contained = true;
                    break;
                }
            }

            if (contained) continue;

            triangles.push_back({prev, curr, next});

            remaining.erase(remaining.begin() + i);

            clipped = true;
            break;
        }

        if (!clipped)
        {
            triangles.clear();
            for (size_t i = 1; i + 1 < v.size(); i++)
            {
                triangles.push_back({0, i, i + 1});
            }
            return triangles;
        }
    }

    if (remaining.size() == 3)
    {
        triangles.push_back({remaining[0], remaining[1], remaining[2]});
    }

    return triangles;
}
