#ifndef SQUARE_H
#define SQUARE_H
#include "Vec3.h"
#include <vector>
#include "Mesh.h"
#include <cmath>

struct RaySquareIntersection
{
    bool intersectionExists;
    float t;
    float u, v;
    Vec3 intersection;
    Vec3 normal;
};

class Square : public Mesh
{
public:
    Vec3 m_normal;
    Vec3 m_bottom_left;
    Vec3 m_right_vector;
    Vec3 m_up_vector;

    Square() : Mesh() {}
    Square(Vec3 const &bottomLeft, Vec3 const &rightVector, Vec3 const &upVector, float width = 1., float height = 1.,
           float uMin = 0.f, float uMax = 1.f, float vMin = 0.f, float vMax = 1.f) : Mesh()
    {
        setQuad(bottomLeft, rightVector, upVector, width, height, uMin, uMax, vMin, vMax);
    }

    void setQuad(Vec3 const &bottomLeft, Vec3 const &rightVector, Vec3 const &upVector, float width = 1., float height = 1.,
                 float uMin = 0.f, float uMax = 1.f, float vMin = 0.f, float vMax = 1.f)
    {
        m_right_vector = rightVector;
        m_up_vector = upVector;
        m_normal = Vec3::cross(rightVector, upVector);
        m_bottom_left = bottomLeft;

        m_normal.normalize();
        m_right_vector.normalize();
        m_up_vector.normalize();

        m_right_vector = m_right_vector * width;
        m_up_vector = m_up_vector * height;

        vertices.clear();
        vertices.resize(4);
        vertices[0].position = bottomLeft;
        vertices[0].u = uMin;
        vertices[0].v = vMin;
        vertices[1].position = bottomLeft + m_right_vector;
        vertices[1].u = uMax;
        vertices[1].v = vMin;
        vertices[2].position = bottomLeft + m_right_vector + m_up_vector;
        vertices[2].u = uMax;
        vertices[2].v = vMax;
        vertices[3].position = bottomLeft + m_up_vector;
        vertices[3].u = uMin;
        vertices[3].v = vMax;
        vertices[0].normal = vertices[1].normal = vertices[2].normal = vertices[3].normal = m_normal;
        triangles.clear();
        triangles.resize(2);
        triangles[0][0] = 0;
        triangles[0][1] = 1;
        triangles[0][2] = 2;
        triangles[1][0] = 0;
        triangles[1][1] = 2;
        triangles[1][2] = 3;
    }

    void updatePositions()
    {
        m_bottom_left = vertices[0].position;
        m_right_vector = vertices[1].position - m_bottom_left;
        m_up_vector = vertices[3].position - m_bottom_left;
        m_normal = vertices[0].normal;
    }
    void scale(Vec3 const &scale)
    {
        Mesh::scale(scale);
        updatePositions();
    }
    void translate(Vec3 const &translation)
    {
        Mesh::translate(translation);
        updatePositions();
    }
    void rotate_x(float angle)
    {
        Mesh::rotate_x(angle);
        updatePositions();
    }
    void rotate_y(float angle)
    {
        Mesh::rotate_y(angle);
        updatePositions();
    }
    void rotate_z(float angle)
    {
        Mesh::rotate_z(angle);
        updatePositions();
    }

    RaySquareIntersection intersect(const Ray &ray) const
    {
        RaySquareIntersection intersection;

        // TODO calculer l'intersection rayon quad
        Vec3 o = ray.origin();
        Vec3 d = ray.direction();
        Vec3 n = m_normal;
        Vec3 a = m_bottom_left;

        float D = Vec3::dot(a, n);

        float denom = Vec3::dot(d, n);
        if (fabs(denom) < 1e-6f)
        {
            intersection.intersectionExists = false;
            return intersection;
        }

        float t = (D - Vec3::dot(o, n)) / denom;
        if (t < 0.0f)
        {
            intersection.intersectionExists = false;
            return intersection;
        }

        Vec3 p = o + t * d;
        Vec3 pa = p - a;

        Vec3 haut = m_up_vector;
        Vec3 droite = m_right_vector;

        float hauteur = haut.length();    // ||haut||
        float longueur = droite.length(); // ||droite||

        haut.normalize();
        droite.normalize();

        float u = Vec3::dot(pa, droite) / longueur; // (pa.droite) / ||droite||
        float v = Vec3::dot(pa, haut) / hauteur;

        if ((u < 0.0f || u > 1.0f) || (v < 0.0f || v > 1.0f))
        {
            intersection.intersectionExists = false;
            return intersection;
        }
        intersection.intersectionExists = true;
        intersection.t = t;
        intersection.intersection = p;
        intersection.normal = n;
        intersection.normal.normalize();
        intersection.u = u;
        intersection.v = v;

        return intersection;
    }
};
#endif // SQUARE_H
