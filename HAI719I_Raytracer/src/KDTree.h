#ifndef KDTREE_H
#define KDTREE_H

#include <vector>
#include "Primitive.h"
#include "RaySceneIntersection.h"

struct KDNode
{
    AABB box;
    int left = -1;
    int right = -1;
    std::vector<int> primIndices;

    bool isLeaf() const { return left == -1 && right == -1; }
};

class KDTree
{
public:
    void build(const std::vector<Primitive> &primitives);
    bool intersect(const Ray &ray,
                   RaySceneIntersection &intersection,
                   const std::vector<Mesh> &meshes,
                   const std::vector<Sphere> &spheres,
                   const std::vector<Square> &squares) const;

private:
    int buildNode(const std::vector<int> &indices, int depth);

    void intersectNode(int nodeIdx, const Ray &ray, RaySceneIntersection &best,
                       const std::vector<Mesh> &meshes,
                       const std::vector<Sphere> &spheres,
                       const std::vector<Square> &squares) const;

    std::vector<KDNode> nodes;
    std::vector<Primitive> prims;
};

#endif