#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "Vec3.h"
#include "Ray.h"
#include "Plane.h"

struct RayTriangleIntersection
{
    bool intersectionExists;
    float t;
    float w0, w1, w2; // coeff barycentre
    unsigned int tIndex;
    Vec3 intersection;
    Vec3 normal;
};

class Triangle
{
private:
    Vec3 m_c[3], m_normal;
    float area;

public:
    Triangle() {}
    Triangle(Vec3 const &c0, Vec3 const &c1, Vec3 const &c2)
    {
        m_c[0] = c0;
        m_c[1] = c1;
        m_c[2] = c2;
        updateAreaAndNormal();
    }
    void updateAreaAndNormal()
    {
        Vec3 nNotNormalized = Vec3::cross(m_c[1] - m_c[0], m_c[2] - m_c[0]);
        float norm = nNotNormalized.length();
        m_normal = nNotNormalized / norm;
        area = norm / 2.f;
    }
    void setC0(Vec3 const &c0) { m_c[0] = c0; } // remember to update the area and normal afterwards!
    void setC1(Vec3 const &c1) { m_c[1] = c1; } // remember to update the area and normal afterwards!
    void setC2(Vec3 const &c2) { m_c[2] = c2; } // remember to update the area and normal afterwards!
    Vec3 const &normal() const { return m_normal; }
    Vec3 projectOnSupportPlane(Vec3 const &p) const
    {
        Plane supportPlane(m_c[0], m_normal);
        return supportPlane.project(p);
    }
    float squareDistanceToSupportPlane(Vec3 const &p) const
    {
        Plane supportPlane(m_c[0], m_normal);
        return supportPlane.squareDistance(p);
    }
    float distanceToSupportPlane(Vec3 const &p) const { return sqrt(squareDistanceToSupportPlane(p)); }
    bool isParallelTo(Line const &L) const
    {
        Plane supportPlane(m_c[0], m_normal);
        return supportPlane.isParallelTo(L);
    }
    Vec3 getIntersectionPointWithSupportPlane(Line const &L) const
    {
        Plane supportPlane(m_c[0], m_normal);
        return supportPlane.getIntersectionPoint(L);
    }
    void computeBarycentricCoordinates(Vec3 const &p, float &u0, float &u1, float &u2) const
    {
        const Vec3 &a = m_c[0];
        const Vec3 &b = m_c[1];
        const Vec3 &c = m_c[2];
    }

    RayTriangleIntersection getIntersection(Ray const &ray) const
    {
        RayTriangleIntersection result;
        // 1) check that the ray is not parallel to the triangle:

        Vec3 o = ray.origin();
        Vec3 d = ray.direction();

        // 2) check that the triangle is "in front of" the ray:

        // 3) check that the intersection point is inside the triangle:
        // CONVENTION: compute u,v such that p = w0*c0 + w1*c1 + w2*c2, check that 0 <= w0,w1,w2 <= 1

        // 4) Finally, if all conditions were met, then there is an intersection! :

        return result;
    }
};
#endif
