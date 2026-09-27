#ifndef AABB_H
#define AABB_H

#include "Vec3.h"
#include "Ray.h"
#include <algorithm>

struct AABB
{
    Vec3 min, max;

    bool intersect(const Ray &ray, float tmin, float tmax) const
    {
        Vec3 o = ray.origin();
        Vec3 d = ray.direction();

        for (int i = 0; i < 3; i++)
        {
            float invD = 1.0f / d[i];
            float t0 = (min[i] - o[i]) * invD;
            float t1 = (max[i] - o[i]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if (tmax <= tmin)
                return false;
        }
        return true;
    }
};

#endif
