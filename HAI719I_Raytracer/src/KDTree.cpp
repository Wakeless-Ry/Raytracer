#include "KDTree.h"
#include <algorithm>

static const int MAX_DEPTH = 20;
static const int MIN_PRIMS = 4;

void KDTree::build(const std::vector<Primitive> &primitives)
{
    prims = primitives;
    nodes.clear();

    std::vector<int> indices(prims.size());
    for (int i = 0; i < prims.size(); i++)
    {
        indices[i] = i;
    }
    buildNode(indices, 0);
}
