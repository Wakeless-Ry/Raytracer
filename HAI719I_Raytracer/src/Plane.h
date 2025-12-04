#ifndef PLANE_H
#define PLANE_H
#include "Vec3.h"
#include "Line.h"
class Plane
{
private:
    Vec3 m_center, m_normal;

public:
    Plane() {}
    Plane(Vec3 const &c, Vec3 const &n)
    {
        m_center = c;
        m_normal = n;
        m_normal.normalize();
    }
    void setCenter(Vec3 const &c) { m_center = c; }
    void setNormal(Vec3 const &n)
    {
        m_normal = n;
        m_normal.normalize();
    }
    Vec3 const &center() const { return m_center; }
    Vec3 const &normal() const { return m_normal; }

    Vec3 project(Vec3 const &p) const
    {
        Vec3 po = this->m_center - p;
        float m = Vec3::dot(po, this->m_normal) / this->m_normal.length();
        return p - (m * this->m_normal);
    }

    float squareDistance(Vec3 const &p) const { return (project(p) - p).squareLength(); }
    float distance(Vec3 const &p) const { return sqrt(squareDistance(p)); }
    bool isParallelTo(Line const &L) const
    {
        return fabs(Vec3::dot(L.direction(), this->m_normal)) < 1e-6f;
    }
    Vec3 getIntersectionPoint(Line const &L) const
    {
        float denom = Vec3::dot(L.direction(), this->m_normal);
        if (fabs(denom) < 1e-6f)
        {
            return Vec3();
        }

        Vec3 po = L.origin();
        Vec3 d = L.direction();

        float t = Vec3::dot(this->m_center - po, this->m_normal) / denom;

        return po + d * t;
    }
};

#endif
