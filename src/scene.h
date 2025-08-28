#pragma once
#include <iostream>
#include <vector>

#include "structures.h"

using std::unique_ptr;
using std::vector;

struct Ray {
    Vertex center;
    Vertex direction;

    Ray(Vertex c, Vertex dir) : center(c), direction(dir) {};
    float ComputeT(Vertex point);
};

struct Primitive3D {
    int materialID;
    int emissiveMaterialID;
    // returns t parameter of ray
    virtual bool IntersectWithRay(const Ray &ray, float &t) = 0;
    virtual Vertex ComputeNormal(const Vertex &point) = 0;
    virtual ~Primitive3D() = default;
    Primitive3D() : materialID(-1), emissiveMaterialID(-1) {};
};

struct Sphere : Primitive3D {
    Vertex center;
    float radius;

    Sphere(float x, float y, float z, float r) : center(x, y, z), radius(r) {};
    ~Sphere() override = default; 

    bool IntersectWithRay(const Ray &ray, float &t);
    Vertex ComputeNormal(const Vertex &point);
};

struct Triangle : Primitive3D {
    vector<Vertex> points;
    Triangle(Vertex v1, Vertex v2, Vertex v3);
    ~Triangle() override = default;
    
    bool IntersectWithRay(const Ray &ray, float &t);
    Vertex ComputeNormal(const Vertex &point);
};

struct EmissiveMaterial {
    Pixel emissiveColor;
    Pixel attenuation;
    EmissiveMaterial(float r, float g, float b, float c0, float c1, float c2) : emissiveColor(r, g, b), attenuation(c0, c1, c2) {};
};

struct Material {
    Pixel color;
    float KSpecular;
    float KDiffuse;
    float shininess;
    // transmittance coefficient
    float T;
    // index of refraction
    float IOR;

    Material(float r, float g, float b, float kd, float ks,
             float shine, float T, float ior) : color(r, g, b), KSpecular(ks), KDiffuse(kd), shininess(shine), T(T), IOR(ior) {};
};

struct PointLight {
    Vertex center;
    Pixel color;

    PointLight(float x, float y, float z, float r, float g, float b) : center(x, y, z), color(r, g, b) {};
};

struct EnvironmentMap {
    int width;
    int height;
    unique_ptr<float[]> texels;

    EnvironmentMap(int w, int h) : width(w), height(h), texels(nullptr) {};
};

struct Scene {
    vector<unique_ptr<Primitive3D>> primitivesList;
    unique_ptr<vector<PointLight>> lightsList;
    unique_ptr<vector<Material>> materialsList;
    unique_ptr<vector<EmissiveMaterial>> emissiveMaterialsList;
    unique_ptr<EnvironmentMap> envMap;

    Scene();
    void RestartScene();
};
