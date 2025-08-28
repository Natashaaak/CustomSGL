#include "context.h"
#include <memory>
#include <cmath>

using std::make_unique;

//---------------------------------------------------------------------------
// Transform functions
//---------------------------------------------------------------------------


void sglMatrixMode(sglEMatrixMode mode) {
    if (contextNotInitialized() || calledWithinBeginEnd() || isInvalidEnumValue(mode)) {
        return;
    }

    sceneManager->getCurrentContext().currentMatrixMode = mode;
}

void sglPushMatrix(void) {
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    auto currentMatrixMode = sceneManager->getCurrentContext().currentMatrixMode;
    auto& currentMatrixStack = sceneManager->getCurrentContext().transformationStack->at(currentMatrixMode);

    currentMatrixStack.push_back(Matrix(currentMatrixStack.back()));
}

void sglPopMatrix(void) {
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    auto currentMatrixMode = sceneManager->getCurrentContext().currentMatrixMode;
    auto& currentMatrixStack = sceneManager->getCurrentContext().transformationStack->at(currentMatrixMode);
    if (currentMatrixStack.size() == 1) {
        setErrCode(SGL_STACK_UNDERFLOW);
        return;
    }

    currentMatrixStack.pop_back();
}

void sglLoadIdentity(void) {
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    auto currentMatrixMode = sceneManager->getCurrentContext().currentMatrixMode;
    auto& currentMatrixStack = sceneManager->getCurrentContext().transformationStack->at(currentMatrixMode);
    currentMatrixStack.back() = Matrix();
}

void sglLoadMatrix(const float *matrix) {
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    auto currentMatrixMode = sceneManager->getCurrentContext().currentMatrixMode;
    auto& currentMatrixStack = sceneManager->getCurrentContext().transformationStack->at(currentMatrixMode);
    currentMatrixStack.back() = Matrix(matrix);
}

void multWithCurrentMatrix(Matrix other) {
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    auto currentMatrixMode = sceneManager->getCurrentContext().currentMatrixMode;
    auto& currentMatrixStack = sceneManager->getCurrentContext().transformationStack->at(currentMatrixMode);

    currentMatrixStack.back() = currentMatrixStack.back() * other;
}

void sglMultMatrix(const float *matrix) {
    // validity check is done in multWithCurrentMatrix
    multWithCurrentMatrix(Matrix(matrix));
}

void sglTranslate(float x, float y, float z) {
    Matrix translation ({
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z,
        0, 0, 0, 1
    });

    // validity check is done in multWithCurrentMatrix
    multWithCurrentMatrix(translation);
}

void sglScale(float x, float y, float z) {
    Matrix scale ({
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
    });

    // validity check is done in multWithCurrentMatrix
    multWithCurrentMatrix(scale);
}

void sglRotate2D(float angle, float centerx, float centery) {
    sglTranslate(centerx, centery, 0);

    float cosAngle = cosf(angle);
    float sinAngle = sinf(angle);
    Matrix rotateZ ({
        cosAngle,-sinAngle, 0, 0,
        sinAngle, cosAngle, 0, 0,
               0,        0, 1, 0,
               0,        0, 0, 1
    });
    multWithCurrentMatrix(rotateZ);

    sglTranslate(-centerx, -centery, 0);
}

void sglRotateY(float angle) {
    float cosAngle = cosf(angle);
    float sinAngle = sinf(angle);
    Matrix rotateY ({
        cosAngle,  0, -sinAngle, 0,
               0,  1,        0, 0,
        sinAngle, 0, cosAngle, 0,
               0,  0,        0, 1
    });

    // validity check is done in multWithCurrentMatrix
    multWithCurrentMatrix(rotateY);
}

void sglOrtho(float l, float r, float b, float t, float n, float f) {
    if (l == r || b == t || n == f) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }

    Matrix orthoProjection({
        2 / (r - l),          0,           0, -(r + l) / (r - l),
        0,          2 / (t - b),           0, -(t + b) / (t - b),
        0,                    0,-2 / (f - n), -(f + n) / (f - n),
        0,                    0,           0,                  1
    });
    
    // validity check is done in multWithCurrentMatrix
    multWithCurrentMatrix(orthoProjection);
}

void sglFrustum(float l, float r, float b, float t, float n, float f) {
    if (l == r || b == t || n <= 0 || f <= 0) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }

    Matrix perspectiveProjection ({
        2 * n / (r - l),               0,  (r + l) / (r - l),                    0,
                      0, 2 * n / (t - b),  (t + b) / (t - b),                    0,
                      0,               0, -(f + n) / (f - n), -2 * f * n / (f - n),
                      0,               0,                 -1,                    0
    });

    // validity check is done in multWithCurrentMatrix
    multWithCurrentMatrix(perspectiveProjection);
}

void sglViewport(int x, int y, int width, int height) {
    if (width <= 0 || height <= 0) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    // not done clipping
    float viewportWidth = float(width) / 2.0f;
    float viewportHeight = float(height) / 2.0f;
    Matrix viewport ({
        viewportWidth,              0, 0,  x + viewportWidth,
        0,             viewportHeight, 0, y + viewportHeight,
        0,                          0, 1,                  0,
        0,                          0, 0,                  1
    });

    sceneManager->getCurrentContext().viewportMatrix = viewport;
}
