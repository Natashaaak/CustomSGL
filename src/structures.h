#pragma once

#include <math.h>
#include <memory>
#include <vector>

using std::vector;

#define EPSILON_T 0.01f

// Allows to use integers instead of floats for coordinates
struct alignas(16) ScreenVertex {
    int x, y;
    // used for depth buffer checks
    float z;
    ScreenVertex() : x(0), y(0), z(0) {}
    ScreenVertex(int x, int y, float z) : x(x), y(y), z(z) {}
};

struct Vertex {
    float x, y, z, w;
    Vertex() : x(0), y(0), z(0), w(1) {}
    Vertex(float x, float y) : x(x), y(y), z(0), w(1) {}
    Vertex(float x, float y, float z) : x(x), y(y), z(z), w(1) {}
    Vertex(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vertex(vector<float>* v) : x(v->at(0)), y(v->at(1)), z(v->at(2)), w(v->at(3)) {}

    Vertex operator+(Vertex const& v) const;
    Vertex operator-(Vertex const& v) const;
    Vertex operator*(float const& f) const;
    Vertex operator/(float const& f) const;
    Vertex& operator/=(float const& f);

    void Normalize();
};

float DotProd(Vertex const& v1, Vertex const& v2);

Vertex CrossProd(Vertex const& v1, Vertex const& v2);

struct Pixel {
    float r, g, b;
    Pixel() : r(0), g(0), b(0) {}
    Pixel(float r, float g, float b) : r(r), g(g), b(b) {}
    Pixel operator*(float const& f) const {
        return Pixel(r * f, g * f, b * f);
    }
    Pixel operator+(Pixel const& p) const {
        return Pixel(r + p.r, g + p.g, b + p.b);
    }
    Pixel operator*(Pixel const& p) const {
        return Pixel(r * p.r, g * p.g, b * p.b);
    }

    Pixel& operator+=(Pixel const& p) {
        r += p.r;
        g += p.g;
        b += p.b;
        return *this;
    }
};

struct Matrix {
    vector<float> data;

    Matrix();
    /**
      Our matrix is defined [row-by-row] but user gives us [column-by-column] matrices
      in float[] class. So when processing these matrices, they need to be transposed first.

      @param matrix [in] [column-by-column] defined matrix
      @returns Matrix in [row-by-row] definition
     */
    Matrix(const float* matrix);
    Matrix(vector<float>& otherData) : data(otherData) {};
    Matrix(std::initializer_list<float> list) : data(list) {}

    Matrix operator*(Matrix const& m) const;
    Vertex operator*(Vertex const& v) const;
    Matrix operator/(float w);
    void operator/=(float w);
    int Invert();
};

//---------------------------------------------------------------------------
// Filling structures
//---------------------------------------------------------------------------
struct Edge {
    int topY;
    int bottomY;
    float currentX;
    float stepX;
    float currentZ;
    float stepZ;

    Edge(ScreenVertex c1, ScreenVertex c2);
};

struct alignas(64) FillingStruct {
    vector<Edge> edges;
    vector<Edge> activeEdgeList;
    int maxY;
    int minY;

    FillingStruct(int maxHeight) : maxY(0), minY(maxHeight) {}
};
