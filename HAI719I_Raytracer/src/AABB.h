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
        for (int a = 0; a < 3; a++)
        {
            Vec3 o = ray.origin();
            Vec3 d = ray.direction();

            for (int a = 0; a < 3; a++)
            {
                float invD = 1.0f / d[a];
                float t0 = (min[a] - o[a]) * invD;
                float t1 = (max[a] - o[a]) * invD;
                if (invD < 0.0f)
                    std::swap(t0, t1);
                tmin = t0 > tmin ? t0 : tmin;
                tmax = t1 < tmax ? t1 : tmax;
                if (tmax <= tmin)
                    return false;
            }
            return true;
        }
        return true;
    }
};

#endif
