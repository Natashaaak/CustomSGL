#include "structures.h"

using std::initializer_list;
using std::make_unique;
using std::vector;

float DotProd(Vertex const& v1, Vertex const& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vertex CrossProd(Vertex const& v1, Vertex const& v2) {
    return Vertex(v1.y * v2.z - v1.z * v2.y,
                  v1.z * v2.x - v1.x * v2.z,
                  v1.x * v2.y - v1.y * v2.x,
                  0);
}

//---------------------------------------------------------------------------
// Matrix
//---------------------------------------------------------------------------

Matrix::Matrix() : data(16, 0.0f) {
    data[0] = 1.0f;
    data[5] = 1.0f;
    data[10] = 1.0f;
    data[15] = 1.0f;
}

Matrix::Matrix(const float* matrix) : data(16) {
    const int w = 4;
    for (int i = 0; i < w; ++i) {
        for (int j = 0; j < w; ++j) {
            data[i * w + j] = matrix[j * w + i];
        }
    }
}

Matrix Matrix::operator*(Matrix const& m) const {
    Matrix result;
    auto a = data;
    auto b = m.data;
    auto r = result.data.data();

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0;
            for (int k = 0; k < 4; k++) {
                sum += a[i * 4 + k] * b[k * 4 + j];
            }
            r[i * 4 + j] = sum;
        }
    }

    return result;
}

Vertex Matrix::operator*(Vertex const& v) const {
    const float* m = data.data();
    float x = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
    float y = m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w;
    float z = m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w;
    float w = m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w;
    return Vertex(x, y, z, w);
}

Matrix Matrix::operator/(float w) {
    Matrix result = *this;
    for (float& val : result.data) {
        val /= w;
    }
    return result;
}

void Matrix::operator/=(float w) {
    for (float& val : data) {
        val /= w;
    }
}

inline int Convert2DTo1D(int x, int y) {
    return x + y * 4;
}

// inverse matrix computation using gauss_jacobi method
// source: N.R. in C
// if matrix is regular = computatation successfull = returns 0
// in case of singular matrix returns 1
int Matrix::Invert() {
    int indxc[4], indxr[4], ipiv[4];
    int i, icol, irow, j, k, l, ll, n;
    float big, dum, pivinv, temp;
    // satisfy the compiler
    icol = irow = 0;

    // the size of the matrix
    n = 4;

    for (j = 0; j < n; j++) { /* zero pivots */
        ipiv[j] = 0;
    }

    for (i = 0; i < n; i++) {
        big = 0.0;
        for (j = 0; j < n; j++) {
            if (ipiv[j] != 1) {
                for (k = 0; k < n; k++) {
                    if (ipiv[k] == 0) {
                        if (fabs(data[Convert2DTo1D(k, j)]) >= big) {
                            big = fabs(data[Convert2DTo1D(k, j)]);
                            irow = j;
                            icol = k;
                        }
                    } else {
                        if (ipiv[k] > 1) {
                            return 1; /* singular matrix */
                        }
                    }
                }
            }
        }
        ++(ipiv[icol]);
        if (irow != icol) {
            for (l = 0; l < n; l++) {
                temp = data[Convert2DTo1D(l, icol)];
                data[Convert2DTo1D(l, icol)] = data[Convert2DTo1D(l, irow)];
                data[Convert2DTo1D(l, irow)] = temp;
            }
        }
        indxr[i] = irow;
        indxc[i] = icol;
        if (data[Convert2DTo1D(icol, icol)] == 0.0) {
            return 1; /* singular matrix */
        }

        pivinv = 1.0f / data[Convert2DTo1D(icol, icol)];
        data[Convert2DTo1D(icol, icol)] = 1.0;
        for (l = 0; l < n; l++) {
            data[Convert2DTo1D(l, icol)] = data[Convert2DTo1D(l, icol)] * pivinv;
        }

        for (ll = 0; ll < n; ll++) {
            if (ll != icol) {
                dum = data[Convert2DTo1D(icol, ll)];
                data[Convert2DTo1D(icol, ll)] = 0.0;
                for (l = 0; l < n; l++) {
                    data[Convert2DTo1D(l, ll)] = data[Convert2DTo1D(l, ll)] - data[Convert2DTo1D(l, icol)] * dum;
                }
            }
        }
    }
    for (l = n; l--;) {
        if (indxr[l] != indxc[l]) {
            for (k = 0; k < n; k++) {
                temp = data[Convert2DTo1D(indxr[l], k)];
                data[Convert2DTo1D(indxr[l], k)] = data[Convert2DTo1D(indxc[l], k)];
                data[Convert2DTo1D(indxc[l], k)] = temp;
            }
        }
    }

    return 0;  // matrix is regular .. inversion has been succesfull
}

//---------------------------------------------------------------------------
// Vertex
//---------------------------------------------------------------------------

Vertex Vertex::operator+(Vertex const& v) const {
    return Vertex(x + v.x, y + v.y, z + v.z, w + v.w);
}

Vertex Vertex::operator-(Vertex const& v) const {
    return Vertex(x - v.x, y - v.y, z - v.z, w - v.w);
}

Vertex Vertex::operator*(float const& scalar) const {
    return Vertex(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vertex Vertex::operator/(float const& scalar) const {
    return Vertex(x / scalar, y / scalar, z / scalar, w / scalar);
}

Vertex& Vertex::operator/=(float const& scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

void Vertex::Normalize() {
    float scale = sqrt(x * x + y * y + z * z + w * w);
    if (scale > 0.0f) {
        float invScale = 1.0f / scale;
        x *= invScale;
        y *= invScale;
        z *= invScale;
        w *= invScale;
    }
}

//---------------------------------------------------------------------------
// Edge
//---------------------------------------------------------------------------

Edge::Edge(ScreenVertex c1, ScreenVertex c2) {
    topY = c1.y;
    bottomY = c2.y;
    currentX = c1.x;
    currentZ = c1.z;

    float height = topY - bottomY;
    if (height != 0) {  // Prevent division by zero
        stepX = (c2.x - c1.x) / height;
        stepZ = (c2.z - c1.z) / height;
    } else {
        stepX = stepZ = 0;
    }
}
