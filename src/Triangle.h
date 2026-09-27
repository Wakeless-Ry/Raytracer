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
        u0 = Vec3::cross(m_c[1] - p, m_c[2] - p).length() / (2.f * area);
        u1 = Vec3::cross(m_c[2] - p, m_c[0] - p).length() / (2.f * area);
        u2 = Vec3::cross(m_c[0] - p, m_c[1] - p).length() / (2.f * area);
    }

    RayTriangleIntersection getIntersection(Ray const &ray, Vec3 normal0, Vec3 normal1, Vec3 normal2) const
    {
        RayTriangleIntersection result;
        result.intersectionExists = false;
        // 1) check that the ray is not parallel to the triangle:

        Plane supportPlane(m_c[0], m_normal);

        if (supportPlane.isParallelTo(ray))
            return result;

        // 2) check that the triangle is "in front of" the ray:
        Vec3 p = supportPlane.getIntersectionPoint(ray);
        float t = Vec3::dot(p - ray.origin(), ray.direction());
        if (t < 0.f)
            return result;

        // 3) check that the intersection point is inside the triangle:

        float u0, u1, u2;
        computeBarycentricCoordinates(p, u0, u1, u2);

        // CONVENTION: compute u,v such that p = w0*c0 + w1*c1 + w2*c2, check that 0 <= w0,w1,w2 <= 1
        if (u0 < 0.f || u1 < 0.f || u2 < 0.f || (u0 + u1 + u2) > 1.f + 0.000001f)
            return result;

        // 4) Finally, if all conditions were met, then there is an intersection! :

        result.intersectionExists = true;
        result.t = t;
        result.w0 = u0;
        result.w1 = u1;
        result.w2 = u2;
        result.intersection = p + m_normal * 0.0001f;
        result.normal = result.w0 * normal0 + result.w1 * normal1 + result.w2 * normal2;
        result.normal.normalize();

        return result;
    }
};
#endif
