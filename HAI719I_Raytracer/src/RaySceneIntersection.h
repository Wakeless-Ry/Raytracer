#ifndef RAY_SCENE_INTERSECTION_H
#define RAY_SCENE_INTERSECTION_H

#include <cfloat>
#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"

struct RaySceneIntersection
{
    bool intersectionExists;
    unsigned int typeOfIntersectedObject;
    unsigned int objectIndex;
    float t;

    RayTriangleIntersection rayMeshIntersection;
    RaySphereIntersection raySphereIntersection;
    RaySquareIntersection raySquareIntersection;

    RaySceneIntersection() : intersectionExists(false), t(FLT_MAX) {}
};

#endif
