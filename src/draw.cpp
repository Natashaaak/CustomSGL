#include "context.h"
#include "draw_utils.h"
#include "draw_utils.h"
#include "cmath"
#include <memory>
#include <iostream>
#include <vector>

using std::unique_ptr;
using std::make_unique;
using std::vector;
using std::cout;
using std::endl;

//---------------------------------------------------------------------------
// Drawing functions
//---------------------------------------------------------------------------


void sglClear(unsigned what) {  
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    auto &context = sceneManager->getCurrentContext();

    bool clearColor = (what & SGL_COLOR_BUFFER_BIT) == SGL_COLOR_BUFFER_BIT;
    bool clearDepth = (what & SGL_DEPTH_BUFFER_BIT) == SGL_DEPTH_BUFFER_BIT;
    
    if (what & ~(SGL_COLOR_BUFFER_BIT | SGL_DEPTH_BUFFER_BIT)) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }
    
	if (clearColor) {
		std::fill(
            context.colorBuffer->begin(), 
            context.colorBuffer->end(), 
            context.clearColor
        );
    }
    if (clearDepth) {
        std::fill(
            context.depthBuffer->begin(), 
            context.depthBuffer->end(), 
            std::numeric_limits<float>::infinity()
        );    

    }
}

void sglBegin(sglEElementType mode) {
    if (calledWithinBeginEnd() || isInvalidEnumValue(mode)) {
        return;
    }    
    SGLContext& context = sceneManager->getCurrentContext();
    context.currentPrimitiveMode = mode;
    context.insideBegin = true;
    context.screenVertices->clear();
    context.verticesList->clear();
    if (!context.insideBeginScene) {
        recalculateVPMMatrix();
        setScaleFactor();
    }
}

void sglEnd(void) {
    if (calledOutsideBeginEnd()) {
        return;
    }

    SGLContext& context = sceneManager->getCurrentContext();
    auto& vertList = *(context.verticesList);
    context.insideBegin = false;

    if (context.insideBeginScene) {
        if (vertList.size() >= 3) {
            auto& scene = sceneManager->getCurrentContext().scene;
            Vertex v0 = vertList[0];
            Vertex v1 = vertList[1];
            Vertex v2 = vertList[2];
            unique_ptr<Triangle> tri = make_unique<Triangle>(v0, v1, v2);
            tri->materialID = (int)scene.materialsList->size() - 1;
            scene.primitivesList.push_back(move(tri));
        }
        return;
    }

    // transform vertices 
    context.screenVertices->reserve(vertList.size());
    for(const Vertex &v : vertList)
    {
        Vertex transformed = transformPoint(v);
        context.screenVertices->emplace_back(
            static_cast<int>(transformed.x), 
            static_cast<int>(transformed.y), 
            transformed.z
        );
    }

    // process primitive mode
    switch (context.currentPrimitiveMode) {
        case SGL_POINTS:     
            drawPoints();      
            break;
        case SGL_LINES:      
            drawLines();       
            break;
        case SGL_LINE_STRIP: 
            drawLineStrip();   
            break;
        case SGL_LINE_LOOP:  
            drawLineLoop();    
            break;
        case SGL_POLYGON:    
            switch (context.currentAreaMode) {
                case SGL_POINT: 
                    drawPoints();   
                    break;
                case SGL_LINE:  
                    drawLineLoop(); 
                    break;
                case SGL_FILL:  
                    fillPolygon();  
                    break;
            }
            break;
        default: 
            break;
    }
}

void sglVertex4f(float x, float y, float z, float w) { 
	// TODO: Implement

}

void sglVertex3f(float x, float y, float z) {
    if (!sceneManager->getCurrentContext().insideBegin) {
        setErrCode(SGL_INVALID_OPERATION);
        return;
    }

    sceneManager->getCurrentContext().verticesList->push_back(Vertex(x, y, z));
}

void sglVertex2f(float x, float y) {
    sglVertex3f(x, y, 0);
}

void sglCircle(float x, float y, float z, float radius)
{   
    if (contextNotInitialized()) {
        return;
    }
    if (radius <= 0) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }
    
    if (sceneManager->getCurrentContext().currentAreaMode == SGL_POINT) {
        sglBegin(SGL_POINTS);
        sglVertex3f(x, y, z);
        sglEnd();
    }
    else{
        recalculateVPMMatrix();
        setScaleFactor();
        drawBresenhamCircle(x, y, z, radius);
    }
}

void sglEllipse(float cx, float cy, float cz, float a, float b)
{
    if (a <= 0 || b <= 0) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }
    if (contextNotInitialized() || calledWithinBeginEnd()) {
        return;
    }

    if (sceneManager->getCurrentContext().currentAreaMode == SGL_POINT) {
        sglBegin(SGL_POINTS);
        sglVertex3f(cx, cy, cz);
        sglEnd();
        return;
    }

    const int numSegments = 40;
    const float angleStep = 2 * M_PI / (float)numSegments;


    sglBegin(SGL_POLYGON);
    for (int i = 0; i < numSegments; i++)
    {
        float angle = i * angleStep;
        float x = cx + a * cosf(angle);
        float y = cy + b * sinf(angle);
        sglVertex3f(x, y, cz);
    }
    sglEnd();
}

void sglArc(float cx, float cy, float cz, float r, float from, float to) {   
    if (contextNotInitialized()) {
        return;
    }

    if (r <= 0) {
        setErrCode(SGL_INVALID_VALUE);
        return;
    }

    auto areaMode = sceneManager->getCurrentContext().currentAreaMode;
    if (areaMode == SGL_POINT) {
        sglBegin(SGL_POINTS);
        sglVertex3f(cx, cy, cz);
        sglEnd();
        return;
    }

    // make sure that the arc is drawn in the correct direction
    from = fmod(from, 2 * M_PI);
    if (from < 0) {
        from += 2 * M_PI;
    }
    to = fmod(to, 2 * M_PI);
    if (to < 0) {
        to += 2 * M_PI;
    }

    if (from > to) {
        to += 2 * M_PI;
    }

    const int numSegments = std::max(1, int(40 * fabs(to - from) / (2 * M_PI)));
    const float angleStep = (to - from) / numSegments;

    if (areaMode == SGL_FILL) {
        sglBegin(SGL_POLYGON);
        // add center to make circular sector
        sglVertex3f(cx, cy, cz);
    }
    else {
        sglBegin(SGL_LINE_STRIP);
    }

    for (int i = 0; i <= numSegments; i++) {
        // Use direct angle calculation instead of incremental
        double angle = from + i * angleStep;
        // Normalize angle periodically to prevent drift
        if (i % 10 == 0) {
            angle = fmod(angle, 2 * M_PI);
        }
        double x = cx + r * cos(angle);
        double y = cy + r * sin(angle);
        sglVertex3f(float(x), float(y), cz);
    }
    sglEnd();    
}
