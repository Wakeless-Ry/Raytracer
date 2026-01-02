#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"
#include <cmath>
#include <GL/glut.h>
#include <stdlib.h>
#include "KDTree.h"
#include "RaySceneIntersection.h"

enum LightType
{
    LightType_Spherical,
    LightType_Quad
};

struct Light
{
    Vec3 material;
    bool isInCamSpace;
    LightType type;

    Vec3 pos;
    float radius;

    Mesh quad;

    float powerCorrection;

    Light() : powerCorrection(1.0) {}
};

class Scene
{
    std::vector<Mesh> meshes;
    std::vector<Sphere> spheres;
    std::vector<Square> squares;
    std::vector<Light> lights;

    KDTree kdTree;

public:
    Scene()
    {
    }

    void draw()
    {
        // iterer sur l'ensemble des objets, et faire leur rendu :
        for (unsigned int It = 0; It < meshes.size(); ++It)
        {
            Mesh const &mesh = meshes[It];
            mesh.draw();
        }
        for (unsigned int It = 0; It < spheres.size(); ++It)
        {
            Sphere const &sphere = spheres[It];
            sphere.draw();
        }
        for (unsigned int It = 0; It < squares.size(); ++It)
        {
            Square const &square = squares[It];
            square.draw();
        }
    }

    // Ancien compute intersection
    //  RaySceneIntersection computeIntersection(Ray const &ray)
    //  {
    //      RaySceneIntersection result;
    //      // TODO calculer les intersections avec les objets de la scene et garder la plus proche

    //     size_t mesh_size = meshes.size();
    //     for (size_t i = 0; i < mesh_size; i++)
    //     {
    //         Mesh const &m = this->meshes[i];
    //         RayTriangleIntersection intersection = m.intersect(ray);
    //         if (intersection.intersectionExists)
    //         {
    //             if (!result.intersectionExists || result.t > intersection.t)
    //             {
    //                 result.intersectionExists = true;
    //                 result.typeOfIntersectedObject = 0;
    //                 result.objectIndex = i;
    //                 result.t = intersection.t;
    //                 result.rayMeshIntersection = intersection;
    //                 result.rayMeshIntersection.w0 = intersection.w0;
    //                 result.rayMeshIntersection.w1 = intersection.w1;
    //                 result.rayMeshIntersection.w2 = intersection.w2;
    //                 result.rayMeshIntersection.normal = intersection.normal;
    //             }
    //         }
    //     }

    //     size_t sphere_size = spheres.size();
    //     for (size_t i = 0; i < sphere_size; i++)
    //     {
    //         Sphere const &s = this->spheres[i];
    //         RaySphereIntersection intersection = s.intersect(ray);
    //         if (intersection.intersectionExists)
    //         {
    //             if (!result.intersectionExists || result.t > intersection.t)
    //             {
    //                 result.intersectionExists = true;
    //                 result.typeOfIntersectedObject = 1;
    //                 result.objectIndex = i;
    //                 result.t = intersection.t;
    //                 result.raySphereIntersection = intersection;
    //                 result.raySphereIntersection.normal = intersection.normal;
    //             }
    //         }
    //     }

    //     size_t square_size = squares.size();
    //     for (size_t i = 0; i < square_size; i++)
    //     {
    //         Square const &s = this->squares[i];
    //         RaySquareIntersection intersection = s.intersect(ray);
    //         if (intersection.intersectionExists)
    //         {
    //             if (!result.intersectionExists || result.t > intersection.t)
    //             {
    //                 result.intersectionExists = true;
    //                 result.typeOfIntersectedObject = 2;
    //                 result.objectIndex = i;
    //                 result.t = intersection.t;
    //                 result.raySquareIntersection = intersection;
    //                 result.raySquareIntersection.u = intersection.u;
    //                 result.raySquareIntersection.v = intersection.v;
    //                 result.raySquareIntersection.normal = intersection.normal;
    //             }
    //         }
    //     }

    //     return result;
    // }

    RaySceneIntersection computeIntersection(Ray const &ray)
    {
        RaySceneIntersection result;
        kdTree.intersect(ray, result, meshes, spheres, squares);
        return result;
    }

    Vec3 getAmbiante(RaySceneIntersection intersection)
    {
        Vec3 ambiante = Vec3(0., 0., 0.);
        if (intersection.intersectionExists)
        {
            Material material;
            switch (intersection.typeOfIntersectedObject)
            {
            case 0:
                material = this->meshes[intersection.objectIndex].material;
                break;
            case 1:
                material = this->spheres[intersection.objectIndex].material;
                break;
            case 2:
                material = this->squares[intersection.objectIndex].material;
                break;
            }
            ambiante = material.ambient_material;
        }

        return ambiante;
    }

    Vec3 calcul_diffuse(Light light, RaySceneIntersection intersection)
    {
        Vec3 Lp = light.pos;
        Vec3 P = Vec3(0., 0., 0.);
        Vec3 N = Vec3(0., 0., 0.);
        Vec3 light_diffuse = light.material;
        Vec3 material_diffuse = Vec3(0., 0., 0.);

        switch (intersection.typeOfIntersectedObject)
        {
        case 0:
            P = intersection.rayMeshIntersection.intersection;
            N = intersection.rayMeshIntersection.normal;
            material_diffuse = this->meshes[intersection.objectIndex].material.diffuse_material;
            break;
        case 1:
            P = intersection.raySphereIntersection.intersection;
            N = intersection.raySphereIntersection.normal;
            material_diffuse = this->spheres[intersection.objectIndex].material.diffuse_material;
            break;
        case 2:
            P = intersection.raySquareIntersection.intersection;
            N = intersection.raySquareIntersection.normal;
            material_diffuse = this->squares[intersection.objectIndex].material.diffuse_material;
            break;
        }

        Vec3 L = (Lp - P);
        L.normalize();
        float dotNL = std::max(0.f, Vec3::dot(L, N));

        return dotNL * Vec3::compProduct(light_diffuse, material_diffuse);
    }

    Vec3 calcul_speculaire(Light light, RaySceneIntersection intersection, Ray ray)
    {
        Vec3 Lp = light.pos;
        Vec3 P = Vec3(0., 0., 0.);
        Vec3 N = Vec3(0., 0., 0.);
        Vec3 light_spec = light.material;
        Vec3 material_spec = Vec3(0., 0., 0.);

        Vec3 V = -1 * ray.direction();
        double shininess = 1.0;

        switch (intersection.typeOfIntersectedObject)
        {
        case 0:
            P = intersection.rayMeshIntersection.intersection;
            N = intersection.rayMeshIntersection.normal;
            material_spec = this->meshes[intersection.objectIndex].material.specular_material;
            shininess = this->meshes[intersection.objectIndex].material.shininess;
            break;
        case 1:
            P = intersection.raySphereIntersection.intersection;
            N = intersection.raySphereIntersection.normal;
            material_spec = this->spheres[intersection.objectIndex].material.specular_material;
            shininess = this->spheres[intersection.objectIndex].material.shininess;
            break;
        case 2:
            P = intersection.raySquareIntersection.intersection;
            N = intersection.raySquareIntersection.normal;
            material_spec = this->squares[intersection.objectIndex].material.specular_material;
            shininess = this->squares[intersection.objectIndex].material.shininess;
            break;
        }

        Vec3 L = (Lp - P);
        L.normalize();

        float dotLN = std::max(0.f, Vec3::dot(N, L));
        Vec3 R = 2 * dotLN * N - L;

        R.normalize();
        V.normalize();

        float dotRV = std::max(0.f, Vec3::dot(R, V));
        dotRV = pow(dotRV, shininess);

        return dotRV * Vec3::compProduct(light_spec, material_spec);
    }

    float portion_visible(Vec3 point, Light light, float nbSamples = 1.f, float lightRadius = 0.3f)
    {
        float nbOccluded = 0.f;

        if (nbSamples == 1)
        {
            Vec3 L = light.pos - point;
            float lightDistance = L.length();
            L.normalize();

            Vec3 shadowOrigine = point + 0.01f * L;
            Ray shadowRayon(shadowOrigine, L);

            RaySceneIntersection shadowIntersection = computeIntersection(shadowRayon);

            if (shadowIntersection.intersectionExists && shadowIntersection.t < lightDistance && shadowIntersection.t > 0)
                nbOccluded++;
        }
        else
        {
            for (int i = 0; i < nbSamples; i++)
            {
                // Créer un point aléatoire dans un cube autour de mon point lumineux
                Vec3 random_offset(((float)rand() / RAND_MAX - 0.5f) * 2.0f * lightRadius,
                                   ((float)rand() / RAND_MAX - 0.5f) * 2.0f * lightRadius,
                                   ((float)rand() / RAND_MAX - 0.5f) * 2.0f * lightRadius);

                Vec3 randomLightPos = light.pos + random_offset;

                Vec3 L = randomLightPos - point;
                float lightDistance = L.length();
                L.normalize();

                Vec3 shadowOrigine = point + 0.01f * L;
                Ray shadowRayon(shadowOrigine, L);

                RaySceneIntersection shadowIntersection = computeIntersection(shadowRayon);

                if (shadowIntersection.intersectionExists && shadowIntersection.t < lightDistance && shadowIntersection.t > 0)
                    nbOccluded++;
            }
        }

        return 1 - (nbOccluded / nbSamples);
    }

    Vec3 phong(Ray ray, RaySceneIntersection intersection)
    {
        if (!intersection.intersectionExists)
            return Vec3(0., 0., 0.);

        Vec3 ambiante = getAmbiante(intersection);
        Vec3 tmp = Vec3(0., 0., 0.);

        Vec3 P;
        switch (intersection.typeOfIntersectedObject)
        {
        case 0:
            P = intersection.rayMeshIntersection.intersection;
            break;
        case 1:
            P = intersection.raySphereIntersection.intersection;
            break;
        case 2:
            P = intersection.raySquareIntersection.intersection;
            break;
        }

        int n = lights.size();
        for (int i = 0; i < n; i++)
        {
            float visibilite = portion_visible(P, lights[i]);
            tmp += visibilite * (calcul_diffuse(lights[i], intersection) + calcul_speculaire(lights[i], intersection, ray));
            // tmp += (calcul_diffuse(lights[i], intersection) + calcul_speculaire(lights[i], intersection, ray));
        }

        return ambiante + tmp;
    }

    Material getMaterial(RaySceneIntersection intersection)
    {
        switch (intersection.typeOfIntersectedObject)
        {
        case 0:
            return this->meshes[intersection.objectIndex].material;
        case 1:
            return this->spheres[intersection.objectIndex].material;
        case 2:
            return this->squares[intersection.objectIndex].material;
        }
    }

    Vec3 refract(const Vec3 &I, const Vec3 &N, float theta)
    {
        float cos1 = -Vec3::dot(N, I);
        float cos2 = 1.f - theta * theta * (1.f - cos1 * cos1);

        if (cos2 < 0.f)
            return Vec3(0., 0., 0.);

        return theta * I + (theta * cos1 - sqrt(cos2)) * N;
    }

    float reflection(const Vec3 &I, const Vec3 &N, float reflection_index)
    {
        float cos_incident = std::clamp((-1) * Vec3::dot(I, N), 0.f, 1.f);
        float theta_incident = 1.f;
        float theta_transmis = reflection_index;

        if (Vec3::dot(I, N) > 0.f)
            std::swap(theta_incident, theta_transmis);

        float R0 = (theta_incident - theta_transmis) / (theta_incident + theta_transmis);
        R0 = R0 * R0;

        return R0 + (1.f - R0) * pow(1.f - cos_incident, 5.f);
    }

    Vec3 rayTraceRecursive(Ray ray, int remainingBounces)
    {
        RaySceneIntersection intersection = computeIntersection(ray);

        if (!intersection.intersectionExists)
            return Vec3(0., 0., 0.);

        Material material = getMaterial(intersection);

        Vec3 P(0.f, 0.f, 0.f);
        Vec3 N(0.f, 0.f, 0.f);

        switch (intersection.typeOfIntersectedObject)
        {
        case 0:
            P = intersection.rayMeshIntersection.intersection;
            N = intersection.rayMeshIntersection.normal;
            break;
        case 1:
            P = intersection.raySphereIntersection.intersection;
            N = intersection.raySphereIntersection.normal;
            break;
        case 2:
            P = intersection.raySquareIntersection.intersection;
            N = intersection.raySquareIntersection.normal;
            break;
        }

        Vec3 color(0.f, 0.f, 0.f);

        switch (material.type)
        {

        case Material_Mirror:
        {
            if (remainingBounces <= 0)
                return Vec3(0., 0., 0.);

            Vec3 d = ray.direction();
            Vec3 rayon = d - 2.f * Vec3::dot(d, N) * N;
            rayon.normalize();

            Ray rayon_reflechie(P + 0.0001f * N, rayon);
            return rayTraceRecursive(rayon_reflechie, remainingBounces - 1);
        }

        case Material_Glass:
        {
            if (remainingBounces <= 0)
                return Vec3(0, 0, 0);

            Vec3 I = ray.direction();
            I.normalize();

            float transparency = material.transparency;

            Vec3 tmp = N;
            bool outside = Vec3::dot(I, N) < 0.f;
            float theta = outside ? (1.f / material.index_medium) : material.index_medium;

            if (!outside)
                tmp = (-1) * N;

            Vec3 reflectDir = I - 2.f * Vec3::dot(I, tmp) * tmp;
            reflectDir.normalize();
            Ray reflectRay(P + 0.0001f * tmp, reflectDir);
            Vec3 reflectColor = rayTraceRecursive(reflectRay, remainingBounces - 1);

            Vec3 refractDir = refract(I, tmp, theta);
            Vec3 refractColor(0, 0, 0);

            if (refractDir.squareLength() > 0)
            {
                refractDir.normalize();
                Ray refractRay(P - 0.0001f * tmp, refractDir);
                refractColor = rayTraceRecursive(refractRay, remainingBounces - 1);
            }

            float coef = reflection(I, tmp, material.index_medium);
            return coef * reflectColor + (1.f - coef) * refractColor;
        }

        default:
            return color = phong(ray, intersection);
        }
    }

    Vec3 rayTrace(Ray const &rayStart)
    {
        return rayTraceRecursive(rayStart, 10);
    }

    void setup_single_sphere()
    {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize(lights.size() + 1);
            Light &light = lights[lights.size() - 1];
            light.pos = Vec3(-5, 5, 5);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1, 1, 1);
            light.isInCamSpace = false;
        }
        {
            spheres.resize(spheres.size() + 1);
            Sphere &s = spheres[spheres.size() - 1];
            s.m_center = Vec3(0., 0., 0.);
            s.m_radius = 1.f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3(1., 1., 1);
            s.material.specular_material = Vec3(0.2, 0.2, 0.2);
            s.material.shininess = 20;
        }

        // Setup KDTree

        std::vector<Primitive> prims;

        for (int i = 0; i < meshes.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_MESH;
            p.index = i;
            p.box = meshes[i].computeAABB();
            prims.push_back(p);
        }

        for (int i = 0; i < spheres.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_SPHERE;
            p.index = i;
            float r = spheres[i].m_radius;
            p.box.min = spheres[i].m_center - Vec3(r, r, r);
            p.box.max = spheres[i].m_center + Vec3(r, r, r);
            prims.push_back(p);
        }

        for (int i = 0; i < squares.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_SQUARE;
            p.index = i;
            p.box = squares[i].computeAABB();
            prims.push_back(p);
        }

        kdTree.build(prims);
    }

    void setup_single_square()
    {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize(lights.size() + 1);
            Light &light = lights[lights.size() - 1];
            light.pos = Vec3(-5, 5, 5);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1, 1, 1);
            light.isInCamSpace = false;
        }

        {
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.build_arrays();
            s.material.diffuse_material = Vec3(0.8, 0.8, 0.8);
            s.material.specular_material = Vec3(0.8, 0.8, 0.8);
            s.material.shininess = 20;
        }

        // Setup KDTree

        std::vector<Primitive> prims;

        for (int i = 0; i < meshes.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_MESH;
            p.index = i;
            p.box = meshes[i].computeAABB();
            prims.push_back(p);
        }

        for (int i = 0; i < spheres.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_SPHERE;
            p.index = i;
            float r = spheres[i].m_radius;
            p.box.min = spheres[i].m_center - Vec3(r, r, r);
            p.box.max = spheres[i].m_center + Vec3(r, r, r);
            prims.push_back(p);
        }

        for (int i = 0; i < squares.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_SQUARE;
            p.index = i;
            p.box = squares[i].computeAABB();
            prims.push_back(p);
        }

        kdTree.build(prims);
    }

    void setup_single_mesh()
    {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize(lights.size() + 1);
            Light &light = lights[lights.size() - 1];
            light.pos = Vec3(0.0, 1.5, 0.0);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1, 1, 1);
            light.isInCamSpace = false;
        }

        { // Back Wall
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.build_arrays();
            s.material.diffuse_material = Vec3(0.5, 0.0, 0.5);
            s.material.specular_material = Vec3(1., 1., 1.);
            s.material.shininess = 16;
        }

        { // Left Wall

            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.rotate_y(90);
            s.build_arrays();
            s.material.diffuse_material = Vec3(1., 0., 0.);
            s.material.specular_material = Vec3(1., 0., 0.);
            s.material.shininess = 16;
        }

        { // Right Wall
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3(0.0, 1.0, 0.0);
            s.material.specular_material = Vec3(0.0, 1.0, 0.0);
            s.material.shininess = 16;
        }

        { // Floor
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3(0.0, 0.0, 1.0);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
        }

        { // Ceiling
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(90);
            s.build_arrays();
            s.material.diffuse_material = Vec3(0.0, 0.5, 0.5);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
        }

        { // Front Wall
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(180);
            s.build_arrays();
            s.material.diffuse_material = Vec3(1.0, 1.0, 1.0);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
        }

        {
            meshes.resize(meshes.size() + 1);
            Mesh &m = meshes.back();
            m.loadOFF("suzanne.off");
            m.centerAndScaleToUnit();
            m.scale(Vec3(0.5, 0.5, 0.5));
            m.translate(Vec3(0.0, -1.25, -1.25));
            m.build_arrays();
            m.material.type = Material_Mirror;
            m.material.diffuse_material = Vec3(0.8f, 0.4f, 1.0f);
            m.material.specular_material = Vec3(0.8f, 0.4f, 1.0f);
            m.material.shininess = 16;
            m.material.transparency = 0.;
            m.material.index_medium = 0.;
        }

        { // Glass Sphere
            spheres.resize(spheres.size() + 1);
            Sphere &s = spheres[spheres.size() - 1];
            // s.m_center = Vec3(-1.0, -1.25, -0.5);
            s.m_center = Vec3(0., 0., 1.0);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3(1., 1., 1.);
            s.material.specular_material = Vec3(1., 1., 1.);
            s.material.shininess = 16;
            s.material.transparency = 0.9f;
            s.material.index_medium = 1.5f;
        }

        // Setup KDTree

        std::vector<Primitive> prims;

        for (int i = 0; i < meshes.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_MESH;
            p.index = i;
            p.box = meshes[i].computeAABB();
            prims.push_back(p);
        }

        for (int i = 0; i < spheres.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_SPHERE;
            p.index = i;
            float r = spheres[i].m_radius;
            p.box.min = spheres[i].m_center - Vec3(r, r, r);
            p.box.max = spheres[i].m_center + Vec3(r, r, r);
            prims.push_back(p);
        }

        for (int i = 0; i < squares.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_SQUARE;
            p.index = i;
            p.box = squares[i].computeAABB();
            prims.push_back(p);
        }

        kdTree.build(prims);
    }

    void setup_cornell_box()
    {
        meshes.clear();
        spheres.clear();
        squares.clear();
        lights.clear();

        {
            lights.resize(lights.size() + 1);
            Light &light = lights[lights.size() - 1];
            light.pos = Vec3(0.0, 1.5, 0.0);
            light.radius = 2.5f;
            light.powerCorrection = 2.f;
            light.type = LightType_Spherical;
            light.material = Vec3(1, 1, 1);
            light.isInCamSpace = false;
        }

        { // Back Wall
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.build_arrays();
            s.material.diffuse_material = Vec3(0.5, 0.0, 0.5);
            s.material.specular_material = Vec3(1., 1., 1.);
            s.material.shininess = 16;
        }

        { // Left Wall

            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.scale(Vec3(2., 2., 1.));
            s.translate(Vec3(0., 0., -2.));
            s.rotate_y(90);
            s.build_arrays();
            s.material.diffuse_material = Vec3(1., 0., 0.);
            s.material.specular_material = Vec3(1., 0., 0.);
            s.material.shininess = 16;
        }

        { // Right Wall
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3(0.0, 1.0, 0.0);
            s.material.specular_material = Vec3(0.0, 1.0, 0.0);
            s.material.shininess = 16;
        }

        { // Floor
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(-90);
            s.build_arrays();
            s.material.diffuse_material = Vec3(0.0, 0.0, 1.0);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
        }

        { // Ceiling
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_x(90);
            s.build_arrays();
            s.material.diffuse_material = Vec3(0.0, 0.5, 0.5);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
        }

        { // Front Wall
            squares.resize(squares.size() + 1);
            Square &s = squares[squares.size() - 1];
            s.setQuad(Vec3(-1., -1., 0.), Vec3(1., 0, 0.), Vec3(0., 1, 0.), 2., 2.);
            s.translate(Vec3(0., 0., -2.));
            s.scale(Vec3(2., 2., 1.));
            s.rotate_y(180);
            s.build_arrays();
            s.material.diffuse_material = Vec3(1.0, 1.0, 1.0);
            s.material.specular_material = Vec3(1.0, 1.0, 1.0);
            s.material.shininess = 16;
        }

        // { // MIRRORED Sphere

        //     spheres.resize(spheres.size() + 1);
        //     Sphere &s = spheres[spheres.size() - 1];
        //     s.m_center = Vec3(1.0, -1.25, 0.5);
        //     s.m_radius = 0.75f;
        //     s.build_arrays();
        //     s.material.type = Material_Mirror;
        //     s.material.diffuse_material = Vec3(1., 0., 0.);
        //     s.material.specular_material = Vec3(1., 0., 0.);
        //     s.material.shininess = 16;
        // }

        { // Glass Sphere
            spheres.resize(spheres.size() + 1);
            Sphere &s = spheres[spheres.size() - 1];
            // s.m_center = Vec3(-1.0, -1.25, -0.5);
            s.m_center = Vec3(0., 0., -0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3(1., 1., 1.);
            s.material.specular_material = Vec3(1., 1., 1.);
            s.material.shininess = 16;
            s.material.transparency = 0.9f;
            s.material.index_medium = 1.5f;
        }

        // Setup KDTree

        std::vector<Primitive> prims;

        for (int i = 0; i < meshes.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_MESH;
            p.index = i;
            p.box = meshes[i].computeAABB();
            prims.push_back(p);
        }

        for (int i = 0; i < spheres.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_SPHERE;
            p.index = i;
            float r = spheres[i].m_radius;
            p.box.min = spheres[i].m_center - Vec3(r, r, r);
            p.box.max = spheres[i].m_center + Vec3(r, r, r);
            prims.push_back(p);
        }

        for (int i = 0; i < squares.size(); i++)
        {
            Primitive p;
            p.type = PRIMITIVE_SQUARE;
            p.index = i;
            p.box = squares[i].computeAABB();
            prims.push_back(p);
        }

        kdTree.build(prims);
    }
};

#endif
