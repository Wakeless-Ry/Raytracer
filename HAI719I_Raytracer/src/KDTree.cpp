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

int KDTree::buildNode(const std::vector<int> &indices, int depth)
{
    KDNode node;
    node.primIndices = indices;

    Vec3 bmin(1e30f, 1e30f, 1e30f);
    Vec3 bmax(-1e30f, -1e30f, -1e30f);

    for (int idx : indices)
    {
        for (int i = 0; i < 3; i++)
        {
            bmin[i] = std::min(bmin[i], prims[idx].box.min[i]);
            bmax[i] = std::max(bmax[i], prims[idx].box.max[i]);
        }
    }
    node.box.min = bmin;
    node.box.max = bmax;

    int nodeIndex = nodes.size();
    nodes.push_back(node);

    if (depth >= MAX_DEPTH || indices.size() <= MIN_PRIMS)
        return nodeIndex;

    int axis = depth % 3;
    float split = 0.5f * (bmin[axis] + bmax[axis]);

    std::vector<int> leftIndices;
    std::vector<int> rightIndices;

    for (int idx : indices)
    {
        float center =
            0.5f * (prims[idx].box.min[axis] + prims[idx].box.max[axis]);

        if (center < split)
            leftIndices.push_back(idx);
        else
            rightIndices.push_back(idx);
    }

    if (leftIndices.empty() || rightIndices.empty())
        return nodeIndex;

    nodes[nodeIndex].left = buildNode(leftIndices, depth + 1);
    nodes[nodeIndex].right = buildNode(rightIndices, depth + 1);

    return nodeIndex;
}
