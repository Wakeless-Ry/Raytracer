#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "AABB.h"

enum PrimitiveType
{
    PRIMITIVE_MESH,
    PRIMITIVE_SPHERE,
    PRIMITIVE_SQUARE
};

struct Primitive
{
    PrimitiveType type;
    int index;
    AABB box;
};

#endif
