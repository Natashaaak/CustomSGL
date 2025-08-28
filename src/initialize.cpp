#include "context.h"
#include <iostream>
#include <memory>

using std::unique_ptr;

unique_ptr<SGLSceneManager> sceneManager;

void sglInit(void) {
    sceneManager = std::make_unique<SGLSceneManager>();
}

void sglFinish(void) {
}

int sglCreateContext(int width, int height) {
    unique_ptr<SGLContext> context = std::make_unique<SGLContext>(width, height);
    sceneManager->contexts.push_back(std::move(context));
    return sceneManager->contexts.size() - 1;
}

bool isValidContextId(size_t id) {
    return id >= 0 && id < sceneManager->contexts.size();
}

void sglDestroyContext(int id) {
    if (!isValidContextId(id)) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }
    if (id == sceneManager->currentContextId) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }
    sceneManager->contexts.erase(sceneManager->contexts.begin() + id);
}

void sglSetContext(int id) {
    if (!isValidContextId(id)) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }
    sceneManager->currentContextId = id;
}

int sglGetContext(void) {
    if (sceneManager->currentContextId == -1) {
        setErrCode(SGL_INVALID_OPERATION);
    }
    return sceneManager->currentContextId;
}

float *sglGetColorBufferPointer(void) {
    if (!isValidContextId(sceneManager->currentContextId)) {
        return nullptr;
    }
    return reinterpret_cast<float *>(sceneManager->getCurrentContext().colorBuffer->data());
}
