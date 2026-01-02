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

Vec3 bmin(1e30f, 1e30f, 1e30f);
Vec3 bmax(-1e30f, -1e30f, -1e30f);

int KDTree::buildNode(const std::vector<int> &indices, int depth)
{
    KDNode node;

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
    {
        nodes[nodeIndex].primIndices = indices;
        return nodeIndex;
    }

    int axis = depth % 3;
    float split = 0.5f * (bmin[axis] + bmax[axis]);

    std::vector<int> leftIndices, rightIndices;
    for (int idx : indices)
    {
        float center = 0.5f * (prims[idx].box.min[axis] + prims[idx].box.max[axis]);
        (center < split ? leftIndices : rightIndices).push_back(idx);
    }

    if (leftIndices.empty() || rightIndices.empty())
    {
        nodes[nodeIndex].primIndices = indices;
        return nodeIndex;
    }

    nodes[nodeIndex].left = buildNode(leftIndices, depth + 1);
    nodes[nodeIndex].right = buildNode(rightIndices, depth + 1);

    return nodeIndex;
}

bool KDTree::intersect(const Ray &ray,
                       RaySceneIntersection &intersection,
                       const std::vector<Mesh> &meshes,
                       const std::vector<Sphere> &spheres,
                       const std::vector<Square> &squares) const
{
    if (nodes.empty())
        return false;

    intersectNode(0, ray, intersection, meshes, spheres, squares);
    return intersection.intersectionExists;
}

void KDTree::intersectNode(int nodeIdx,
                           const Ray &ray,
                           RaySceneIntersection &best,
                           const std::vector<Mesh> &meshes,
                           const std::vector<Sphere> &spheres,
                           const std::vector<Square> &squares) const
{
    const KDNode &node = nodes[nodeIdx];

    if (!node.box.intersect(ray, 0.001f, best.t))
        return;

    if (node.isLeaf())
    {
        for (int primIdx : node.primIndices)
        {
            const Primitive &p = prims[primIdx];

            if (p.type == PRIMITIVE_MESH)
            {
                RayTriangleIntersection h =
                    meshes[p.index].intersect(ray);

                if (h.intersectionExists && h.t < best.t)
                {
                    best.intersectionExists = true;
                    best.typeOfIntersectedObject = 0;
                    best.objectIndex = p.index;
                    best.t = h.t;
                    best.rayMeshIntersection = h;
                }
            }
            else if (p.type == PRIMITIVE_SPHERE)
            {
                RaySphereIntersection h =
                    spheres[p.index].intersect(ray);

                if (h.intersectionExists && h.t < best.t)
                {
                    best.intersectionExists = true;
                    best.typeOfIntersectedObject = 1;
                    best.objectIndex = p.index;
                    best.t = h.t;
                    best.raySphereIntersection = h;
                }
            }
            else if (p.type == PRIMITIVE_SQUARE)
            {
                RaySquareIntersection h =
                    squares[p.index].intersect(ray);

                if (h.intersectionExists && h.t < best.t)
                {
                    best.intersectionExists = true;
                    best.typeOfIntersectedObject = 2;
                    best.objectIndex = p.index;
                    best.t = h.t;
                    best.raySquareIntersection = h;
                }
            }
        }
        return;
    }

    intersectNode(node.left, ray, best, meshes, spheres, squares);
    intersectNode(node.right, ray, best, meshes, spheres, squares);
}