#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "AABB.h"

enum PrimitiveType
{
    PRIMITIVE_TRIANGLE,
    PRIMITIVE_SPHERE,
    PRIMITIVE_SQUARE
};

struct Primitive
{
    PrimitiveType type;
    int meshIndex;
    int triangleIndex;
    int index;

    AABB box;
};

#endif
