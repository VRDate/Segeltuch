#ifndef GridH
#define GridH 1

#include "Geometry.h"

class Grid {
protected:
    Vector pos;   //world location
    Vector norm;  //y
    Vector right; //x
    Vector dir;   //z

    float size, step, dot, line;

public:
    Grid() :
        size(100.0), step(1.0), dot(2.0), line(10.0),
        right(1.0, 0.0, 0.0),
        norm(0.0, 1.0, 0.0),
        dir(0.0, 0.0, 1.0) {}

    inline const Vector &GetNormal() { return norm; }
    inline const Vector &GetPos() { return pos; }

    void SetPosition(const Vector &p)
    {
        pos = p;
    }

    void SetNormal(const Vector &n)
    {
        if (n.length() < 0.001) return;
        norm = n;
        norm.normalize();
    }

    void SetRight(const Vector &r)
    {
        if (r.length() < 0.001) return;
        right = r;
        right.normalize();
        Vector::cross(dir, norm, right);
    }

    void SetDir(const Vector &d)
    {
        if (d.length() < 0.001) return;
        dir = d;
        dir.normalize();
        Vector::cross(dir, norm, right);
    }

    void GetNearestPoint(Vector &pt)
    {
        float invs, ld;
        float px, py;

        invs = 1.0/step;

        // Project onto the grid
        px = Vector::dot(right, pt - pos);
        py = Vector::dot(dir, pt - pos);

        // Snap to the closest grid step
        px = ((int)(px*invs+0.5))*step;
        py = ((int)(py*invs+0.5))*step;

        // Bring back into 3D
        pt.x = pos.x + (px*right.x)+(py*dir.x);
        pt.y = pos.y + (px*right.y)+(py*dir.y);
        pt.z = pos.z + (px*right.z)+(py*dir.z);
    }

    void Draw();
};

#endif //GridH
