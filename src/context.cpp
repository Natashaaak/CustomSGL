#include "context.h"
#include "structures.h"
#include <iostream>
#include <memory>
#include <vector>

using std::make_unique;
using std::initializer_list;
using std::vector;


SGLContext::SGLContext(int width, int height) : 
  width(width), 
  height(height), 
  currentPrimitiveMode(SGL_POINTS),
  currentAreaMode(SGL_FILL),
  currentMatrixMode(SGL_MODELVIEW),
  pointSize(1),
  scaleFactor(1),
  insideBegin(false),
  enabledDepthTest(true),
  insideBeginScene(false) {
    colorBuffer = make_unique<vector<Pixel>>(width * height); 
    depthBuffer = make_unique<vector<float>>(width * height, 1.0f);
    transformationStack = make_unique<vector<vector<Matrix>>>(2);
    transformationStack->at(0).push_back(Matrix());
    transformationStack->at(1).push_back(Matrix());
    screenVertices = make_unique<vector<ScreenVertex>>();
    verticesList = make_unique<vector<Vertex>>();
}

SGLSceneManager::SGLSceneManager() :
  currentContextId(-1), 
  errorCode(SGL_NO_ERROR) {
}

//---------------------------------------------------------------------------
// Check utils
//---------------------------------------------------------------------------

bool contextNotInitialized() {
    if (!sceneManager || !sceneManager->contexts[sceneManager->currentContextId]) {
        setErrCode(SGL_INVALID_OPERATION);
        return true;
    }
    return false;
}

bool calledWithinBeginEnd() {
    if (sceneManager->getCurrentContext().insideBegin) {
        setErrCode(SGL_INVALID_OPERATION);
        return true;
    }
    return false;
}

bool calledOutsideBeginEnd() {
    if (sceneManager->getCurrentContext().insideBegin == false) {
        setErrCode(SGL_INVALID_OPERATION);
        return true;
    }
    return false;
}

bool calledWithinBeginSceneEndScene()
{
    if (sceneManager->getCurrentContext().insideBeginScene) {
        setErrCode(SGL_INVALID_OPERATION);
        return true;
    }
    return false;
}

bool calledOutsideBeginSceneEndScene()
{
    if (sceneManager->getCurrentContext().insideBeginScene == false) {
        setErrCode(SGL_INVALID_OPERATION);
        return true;
    }
    return false;
}

void recalculateVPMMatrix() {
    auto& context = sceneManager->getCurrentContext();
    Matrix PM = 
        context.transformationStack->at(SGL_PROJECTION).back() *
        context.transformationStack->at(SGL_MODELVIEW).back();

    float w = PM.data.back();
    if (w != 1) {
        PM = PM / w;
    }
    context.VPMmatrix = context.viewportMatrix * PM;
}

