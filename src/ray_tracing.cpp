#include "context.h"

//---------------------------------------------------------------------------
// RayTracing oriented functions
//---------------------------------------------------------------------------

void sglBeginScene() {
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    sceneManager->getCurrentContext().scene.RestartScene();
    sceneManager->getCurrentContext().insideBeginScene = true;
}

void sglEndScene() {
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    sceneManager->getCurrentContext().insideBeginScene = false;
}

void sglSphere(const float x,
               const float y,
               const float z,
               const float radius) {
    if (contextNotInitialized() || calledWithinBeginEnd() || calledOutsideBeginSceneEndScene()) {
        return;
    }

    auto& scene = sceneManager->getCurrentContext().scene;

    unique_ptr<Sphere> sphere = make_unique<Sphere>(x, y, z, radius);
    sphere->materialID = static_cast<int>(scene.materialsList->size()) - 1;

    scene.primitivesList.push_back(move(sphere));
}

void sglMaterial(const float r,
                 const float g,
                 const float b,
                 const float kd,
                 const float ks,
                 const float shine,
                 const float T,
                 const float ior) {
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    Material mat = Material(r, g, b, kd, ks, shine, T, ior);

    sceneManager->getCurrentContext().scene.materialsList->push_back(mat);
}

void sglPointLight(const float x,
                   const float y,
                   const float z,
                   const float r,
                   const float g,
                   const float b) {
    if (contextNotInitialized() || calledWithinBeginEnd() || calledOutsideBeginSceneEndScene()) {
        return;
    }

    PointLight light = PointLight(x, y, z, r, g, b);

    sceneManager->getCurrentContext().scene.lightsList->push_back(light);
}

void recalculateRaytracingVPMMatrix () {
    SGLContext& currentContext = sceneManager->getCurrentContext();
    Matrix projectionMatrix = currentContext.transformationStack->at(SGL_PROJECTION).back();
    Matrix modelViewMatrix = currentContext.transformationStack->at(SGL_MODELVIEW).back();
    currentContext.VPMmatrix = projectionMatrix * modelViewMatrix;
}

void castRay(int x, int y, int w, const Matrix& invVPM) {
    auto& currentContext = sceneManager->getCurrentContext();
    
    Ray ray = generatePrimaryRay(x + 0.5f, y + 0.5f, invVPM);
    Pixel color = traceRay(ray, 0);
    currentContext.colorBuffer->at(x + y * w) = color;
}

void raycastSegment(int startY, int endY, int w, const Matrix& invVPM) {
    for (int y = startY; y < endY; y++) {
        for (int x = 0; x < w; x++) {
            // here we assume that the current context will not be modified during the raycasting 
            castRay(x, y, w, invVPM);
        }
    }
}

void sglRayTraceScene() {
    if (contextNotInitialized() || calledWithinBeginEnd() || calledWithinBeginSceneEndScene()) {
        return;
    }
    recalculateRaytracingVPMMatrix();
    const int width = sceneManager->getCurrentContext().width;
    const int height = sceneManager->getCurrentContext().height;

    Matrix invVPM = sceneManager->getCurrentContext().VPMmatrix;
    if (invVPM.Invert()) {
        std::cerr << "Unable to invert VPM matrix" << std::endl;
        return;
    }

    // sequential for debugging
    /*for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            castRay(x, y, width, invVPM);
        }
    }*/

    unsigned int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    
    int rowsPerThread = height / numThreads;
    for (unsigned int i = 0; i < numThreads; i++) {
        int startY = i * rowsPerThread;
        int endY = (i == numThreads - 1) ? height : (i + 1) * rowsPerThread;
        threads.emplace_back(raycastSegment, startY, endY, width, invVPM);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }

    if (USE_ANTIALIASING) {
        antialiase(invVPM);
    }
}

void sglRasterizeScene() {
    // TODO: Implement
}

void sglEnvironmentMap(const int width,
                       const int height,
                       float* texels) {
    if (!texels || contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    auto& currentContext = sceneManager->getCurrentContext();
    currentContext.scene.envMap = make_unique<EnvironmentMap>(width, height);
    const int envMapSize = width * height * 3;
    currentContext.scene.envMap->texels = make_unique<float[]>(envMapSize);
    std::copy(texels, texels + envMapSize, currentContext.scene.envMap->texels.get());
}

void sglEmissiveMaterial(const float r,
                         const float g,
                         const float b,
                         const float c0,
                         const float c1,
                         const float c2) {
    // TODO: Test
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }
    EmissiveMaterial mat = EmissiveMaterial(r, g, b, c0, c1, c2);
    sceneManager->getCurrentContext().scene.emissiveMaterialsList->push_back(mat);
}
