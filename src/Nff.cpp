#include "Nff.h"
#include "Geometry.h"

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
        std::cout << *(static_cast<Triangle*>(_surfaces[i])) << '\n';
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
    Eigen::Vector3d coords;
    std::vector<std::string> failBuffer;
    int vertices = 0;
    Eigen::Vector3d shift;
    

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
                shift = -_at;
                _at << 0,0,0;

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
                if (line[1] == 'p') {ss>>ch; patch = true;}
                ss >> vertices;

                // get the base vertex
                getline(file, line);

                std::vector<Eigen::Vector3d> verts;
                std::vector<Eigen::Vector3d> norms;

                std::stringstream base(line);

                Eigen::Vector3d coords;
                Eigen::Vector3d norm;

                // get the first vertex
                base >> coords[0] >> coords[1] >> coords[2];
                if (patch) 
                {
                    base >> norm[0] >> norm[1] >> norm[2];
                    norms.push_back(norm);
                }

                coords += shift;
                verts.push_back(coords);

                getline(file, line);

                base.str(line);
                base.clear();

                // get vertex 2
                base >> coords[0] >> coords[1] >> coords[2];
                if (patch) 
                {
                    base >> norm[0] >> norm[1] >> norm[2];
                    norms.push_back(norm);
                }

                coords += shift;
                verts.push_back(coords);

                Eigen::Vector3d normalOverride;

                // this will always be the newest vertex
                // follows breaking up any 3 or more vertices polygons by doing
                // 0 1 2 3 4 -> 0 1 2, 0 2 3, 0 3 4
                for (int i = 0; i < vertices - 2; i++)
                {
                    getline(file, line);
                    std::stringstream parser(line);

                    // get newest (3rd) vertex
                    parser >> coords[0] >> coords[1] >> coords[2];
                    coords += shift;
                    verts.push_back(coords);
                    if (patch) 
                    {
                        parser >> norm[0] >> norm[1] >> norm[2];
                        norms.push_back(norm);
                        _surfaces.push_back(new Tripatch(verts, norms, fill));

                        // erase second thing, so 1 2 3 goes 1 3 <ready>
                        norms.erase(std::next(norms.begin()));
                    }
                    else
                    {
                        _surfaces.push_back(new Triangle(verts, fill));
                    }

                    // if we are fanning, get the normal that should be used for the fanned polygons
                    if (i == 0 && vertices > 3) normalOverride = (verts[0]-verts[1]).cross(verts[0]-verts[2]);
                    else if (i > 0)
                    {
                        _surfaces.back()->_normalOverride = true;
                        _surfaces.back()->_normal = normalOverride;
                    }

                    // erase second thing, so 1 2 3 goes 1 3 <ready>
                    // next iteration will fill in ready so it becomes 1 3 4
                    verts.erase(std::next(verts.begin()));
                }
                break;
            }
            // sphere parsing
            case 's':
                break;
                // disabled for the time being
                // ss >> coords[0] >> coords[1] >> coords[2] >> radius;
                // _surfaces.push_back(new Sphere(coords, radius, fill));
                // break;

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