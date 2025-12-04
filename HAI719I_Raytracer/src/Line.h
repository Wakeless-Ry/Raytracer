#ifndef LINE_H
#define LINE_H

#include <cmath>
#include <iostream>
#include "Vec3.h"
#include "Plane.h"

class Line
{
private:
    Vec3 m_origin, m_direction;

public:
    Line() {}
    Line(Vec3 const &o, Vec3 const &d)
    {
        m_origin = o;
        m_direction = d;
        m_direction.normalize();
    }
    Vec3 &origin() { return m_origin; }
    Vec3 const &origin() const { return m_origin; }
    Vec3 &direction() { return m_direction; }
    Vec3 const &direction() const { return m_direction; }
    Vec3 project(Vec3 const &p) const
    {
        Vec3 op = p - this->m_origin;
        float m = Vec3::dot(op, this->m_direction);
        return m_origin + m_direction * m;
    }
    float squareDistance(Vec3 const &p) const
    {
        return (project(p) - p).squareLength();
    }
    float distance(Vec3 const &p) const
    {
        return sqrt(squareDistance(p));
    }
};

static inline std::ostream &operator<<(std::ostream &s, Line const &l)
{
    s << l.origin() << " " << l.direction();
    return s;
}

#endif
