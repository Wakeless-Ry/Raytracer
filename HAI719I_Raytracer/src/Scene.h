#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <string>
#include "Mesh.h"
#include "Sphere.h"
#include "Square.h"
#include <cmath>
#include <GL/glut.h>

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

class Scene
{
    std::vector<Mesh> meshes;
    std::vector<Sphere> spheres;
    std::vector<Square> squares;
    std::vector<Light> lights;

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

    RaySceneIntersection computeIntersection(Ray const &ray)
    {
        RaySceneIntersection result;
        // TODO calculer les intersections avec les objets de la scene et garder la plus proche

        size_t sphere_size = spheres.size();
        for (size_t i = 0; i < sphere_size; i++)
        {
            Sphere s = this->spheres[i];
            RaySphereIntersection intersection = s.intersect(ray);
            if (intersection.intersectionExists)
            {
                if (!result.intersectionExists || result.raySphereIntersection.t < intersection.t)
                {
                    result.intersectionExists = true;
                    result.typeOfIntersectedObject = 1;
                    result.objectIndex = i;
                    result.t = intersection.t;
                    result.raySphereIntersection = intersection;
                }
            }
        }

        size_t square_size = squares.size();
        for (size_t i = 0; i < square_size; i++)
        {
            Square s = this->squares[i];
            RaySquareIntersection intersection = s.intersect(ray);
            if (intersection.intersectionExists)
            {
                if (!result.intersectionExists || result.raySquareIntersection.t < intersection.t)
                {
                    result.intersectionExists = true;
                    result.typeOfIntersectedObject = 2;
                    result.objectIndex = i;
                    result.t = intersection.t;
                    result.raySquareIntersection = intersection;
                    result.raySquareIntersection.u = intersection.u;
                    result.raySquareIntersection.v = intersection.v;
                }
            }
        }

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

    bool isInShadow(Vec3 point, Light light)
    {
        Vec3 L = light.pos - point;
        float lightDistance = L.length();
        L.normalize();

        Vec3 shadowOrigine = point + 0.001f * L;
        Ray shadowRayon(shadowOrigine, L);

        RaySceneIntersection shadowIntersection = computeIntersection(shadowRayon);

        if (shadowIntersection.intersectionExists && shadowIntersection.t < lightDistance && shadowIntersection.t > 0)
            return true;

        return false;
    }

    Vec3 phong(Ray ray, RaySceneIntersection intersection)
    {
        Vec3 ambiante = getAmbiante(intersection);
        Vec3 tmp = Vec3(0., 0., 0.);

        Vec3 P;
        switch (intersection.typeOfIntersectedObject)
        {
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
            if (isInShadow(P, lights[i]))
            {
                std::cout << "ombre trouvée " << std::endl;
                continue;
            }
            tmp += calcul_diffuse(lights[i], intersection) + calcul_speculaire(lights[i], intersection, ray);
        }

        return ambiante + tmp;
    }

    Vec3 rayTraceRecursive(Ray ray, int remainingBounces)
    {
        // std::cout << "Computing intersection..." << std::endl;

        Vec3 color = Vec3(0., 0., 0.);
        RaySceneIntersection intersection = computeIntersection(ray);
        color = phong(ray, intersection);

        return color;
    }

    Vec3 rayTrace(Ray const &rayStart)
    {
        return rayTraceRecursive(rayStart, 0);
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

        { // GLASS Sphere

            spheres.resize(spheres.size() + 1);
            Sphere &s = spheres[spheres.size() - 1];
            s.m_center = Vec3(1.0, -1.25, 0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Mirror;
            s.material.diffuse_material = Vec3(1., 0., 0.);
            s.material.specular_material = Vec3(1., 0., 0.);
            s.material.shininess = 16;
            s.material.transparency = 1.0;
            s.material.index_medium = 1.4;
        }

        { // MIRRORED Sphere
            spheres.resize(spheres.size() + 1);
            Sphere &s = spheres[spheres.size() - 1];
            s.m_center = Vec3(-1.0, -1.25, -0.5);
            s.m_radius = 0.75f;
            s.build_arrays();
            s.material.type = Material_Glass;
            s.material.diffuse_material = Vec3(1., 1., 1.);
            s.material.specular_material = Vec3(1., 1., 1.);
            s.material.shininess = 16;
            s.material.transparency = 0.;
            s.material.index_medium = 0.;
        }
    }
};

#endif
