#include "draw_utils.h"
#include <math.h>
#include <algorithm>
#include <functional>

using std::cout;
using std::endl;
using std::make_unique;
using std::max;
using std::min;
using std::swap;

constexpr float EPSILON = 0.000004f;

inline float invZ(float z) {
    return 1.0f / z;
}

inline float interpolateZ(float z1, float z2, float t) {
    return invZ(z1) + t * (invZ(z2) - invZ(z1));
}

inline int coord2DTo1D(int x, int y, int width) {
    return x + y * width;
}

inline float calculateZStep(float z1, float z2, int x1, int x2) {
    if (x2 == x1) {
        return 0;
    } 

    return (invZ(z2) - invZ(z1)) / static_cast<float>(x2 - x1);
}

inline bool depthCheck(ScreenVertex point, int width ) {
    if (!sceneManager->getCurrentContext().enabledDepthTest) {
        return true;
    }
    
    auto& depthBuffer = *(sceneManager->getCurrentContext().depthBuffer);
    int index = coord2DTo1D(point.x, point.y, width);

    if (depthBuffer[index] > point.z - EPSILON) {
        depthBuffer[index] = point.z;  // Update with closer depth
        return true;  // Draw the pixel
    }
    return false;
}

inline bool boundsAndDepthCheck(ScreenVertex point, int width, int height) {
    if (point.x < 0 || point.x >= width || point.y < 0 || point.y >= height) {
        return false;
    }
    if (!sceneManager->getCurrentContext().enabledDepthTest) {
        return true;
    }
    return depthCheck(point, width);
}

inline void plotLine(vector<Pixel>& colorBuffer, Pixel color, int y, int x1, int x2, float z1, float z2, int width) {
    
    if (x1 == x2) {
        if (depthCheck({x1, y, z1}, width)) {
            colorBuffer[coord2DTo1D(x1, y, width)] = color;
        }
        return;
    }

    const float invZStep = calculateZStep(z1, z2, x1, x2);

    float currentInvZ = invZ(z1);
    const int index_base = y * width;

    for (int x = x1; x <= x2; x++) {
        if (depthCheck({x, y, invZ(currentInvZ)}, width)) {
            colorBuffer[index_base+ x] = color;
        }
        currentInvZ += invZStep;
    }
}

inline void plotLineBoundsChecking(vector<Pixel>& colorBuffer, Pixel color, 
                            int y, int x1, int x2, float z1, float z2, 
                            int width ) {
    if (x1 < 0) {
        float currentInvZ = invZ(z1);
        float invZStep = calculateZStep(z1, z2, x1, x2);
        currentInvZ += invZStep * (-x1);
        z1 = 1.0f / currentInvZ;
        x1 = 0;
    }
    if (x2 >= width) {
        x2 = width - 1;
    }
    // the whole line was outside of bounds
    if (x1 > x2) {
        return;
    }

    plotLine(colorBuffer, color, y, x1, x2, z1, z2, width);
}


void drawPoints() {
    auto& context = sceneManager->getCurrentContext();
    auto& screenVertices = *(context.screenVertices);
    auto& colorBuffer = *(context.colorBuffer);
    int width = context.width;
    int height = context.height;
    Pixel color = context.currentColor;
    int size = context.pointSize;

    for (const ScreenVertex &v : screenVertices) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                ScreenVertex pixel = ScreenVertex(v.x + j, v.y + i, v.z);
                if (boundsAndDepthCheck(pixel, width, height)) {
                    colorBuffer[coord2DTo1D(pixel.x, pixel.y, width)] = color;
                }
            }
        }
    }
}

int getIncrement(int start, int end) {
    if (start < end) {
        return 1;
    }
    else if (start > end) {
        return -1;
    }
    return 0;
}

void drawBresenhamLine(ScreenVertex start, ScreenVertex end) {

    // differences
    const int dX = abs(end.x - start.x); 
    const int dY = abs(end.y - start.y); 

    float totalDistance = sqrt(dX * dX + dY * dY);
    if (totalDistance < EPSILON) {
        return;
    }

    // interpolation of z 
    float currentInvZ = invZ(start.z);
    float invZStep = (invZ(end.z) - invZ(start.z)) / (totalDistance > 0 ? totalDistance : 1);


    // direction
    const int sX = (start.x < end.x) ? 1 : -1;
    const int sY = (start.y < end.y) ? 1 : -1;

    int tmp, error = ((dX > dY) ? dX : -dY) / 2;
    
    const int width = sceneManager->getCurrentContext().width;
    const int height = sceneManager->getCurrentContext().height;
    auto& colorBuffer = *(sceneManager->getCurrentContext().colorBuffer);
    const Pixel color = sceneManager->getCurrentContext().currentColor;

    while (start.x != end.x || start.y != end.y) {
        ScreenVertex pixel(start.x, start.y, invZ(currentInvZ));

        if (boundsAndDepthCheck(start, width, height)) {
            colorBuffer[coord2DTo1D(start.x, start.y, width)] = color;
        }
        tmp = error;
        if (tmp > -dX) {
            error -= dY;
            start.x += sX;
            currentInvZ += invZStep * abs(sX);
        }
        if (tmp < dY) {
            error += dX;
            start.y += sY;
            currentInvZ += invZStep * abs(sY);
        }
    }

    ScreenVertex pixel(start.x, start.y, invZ(currentInvZ));
    if(boundsAndDepthCheck(pixel, width, height)) {
        colorBuffer[coord2DTo1D(start.x, start.y, width)] = color;
    }
}

void drawLines() {
    auto& screenVertices = *(sceneManager->getCurrentContext().screenVertices);
    size_t size = screenVertices.size();

    if (size % 2) {
        screenVertices.pop_back();
    }

    for (size_t i = 0; i < size; i+=2) {
        drawBresenhamLine(screenVertices[i], screenVertices[i+1]);
    }
}

void drawLineStrip() {
    auto& screenVertices = *(sceneManager->getCurrentContext().screenVertices);
    size_t primitivesCount = screenVertices.size();
    if (primitivesCount < 2) {
        return;
    }

    for (size_t i = 0; i < primitivesCount - 1; i++) {
        drawBresenhamLine(screenVertices[i], screenVertices[i+1]);
    }
}

void drawLineLoop() {
    auto& screenVertices = *(sceneManager->getCurrentContext().screenVertices);
    size_t primitivesCount = screenVertices.size();
    // impossible to draw a loop with less than 2 vertices
    if (primitivesCount < 2) {
        return;
    }

    for (size_t i = 0; i < primitivesCount - 1; i++) {
        drawBresenhamLine(screenVertices[i], screenVertices[i+1]);
    }
    drawBresenhamLine(screenVertices.back(), screenVertices[0]);
}

Vertex transformPoint(const Vertex& v) {
    Vertex result = sceneManager->getCurrentContext().VPMmatrix * v;
    if (result.w != 0.0f) {
        const float inv_w = 1.0f / result.w;
        result.x *= inv_w;
        result.y *= inv_w;
        result.z *= inv_w;
        result.w = 1.0f;
    }

    result.z = (result.z + 1.0f) * 0.5f;

    return result;
}

void plotTransformedPoint(vector<Pixel>& colorBuffer, Pixel color, ScreenVertex point) {
    int width = sceneManager->getCurrentContext().width;
    int height = sceneManager->getCurrentContext().height;
    
    if (boundsAndDepthCheck(point, width, height)) {
        colorBuffer[coord2DTo1D(point.x, point.y, width)] = color;
    }
}

void setScaleFactor() {
    Matrix VPM = sceneManager->getCurrentContext().VPMmatrix;
    sceneManager->getCurrentContext().scaleFactor = 
        sqrt(VPM.data[0] * VPM.data[5] - VPM.data[1] * VPM.data[4]);
}

void drawBresenhamCircle(float cx, float cy, float cz, float radius) {
    Vertex transformedCenter = transformPoint(Vertex(cx, cy, cz));
    auto& context = sceneManager->getCurrentContext();
    auto width = context.width;

    int centerX = round(transformedCenter.x);
    int centerY = round(transformedCenter.y);
    float depth = transformedCenter.z;
    int r = round(radius * context.scaleFactor);

    auto& colorBuffer = *(context.colorBuffer);
    Pixel color = context.currentColor;

    if (r == 0) {
        plotTransformedPoint(colorBuffer, color, ScreenVertex(centerX, centerY, depth));
        return;
    }

    int x = 0;
    int y = r;
    //  d is the decision variable and is derived from the midpoint of the circle
    int d = 3 - 2 * r;

    auto plotCirclePoints = [&](int x, int y) {
        plotTransformedPoint(colorBuffer, color, ScreenVertex(centerX + x, centerY + y, depth));
        plotTransformedPoint(colorBuffer, color, ScreenVertex(centerX - x, centerY + y, depth));
        plotTransformedPoint(colorBuffer, color, ScreenVertex(centerX + x, centerY - y, depth));
        plotTransformedPoint(colorBuffer, color, ScreenVertex(centerX - x, centerY - y, depth));
        plotTransformedPoint(colorBuffer, color, ScreenVertex(centerX + y, centerY + x, depth));
        plotTransformedPoint(colorBuffer, color, ScreenVertex(centerX - y, centerY + x, depth));
        plotTransformedPoint(colorBuffer, color, ScreenVertex(centerX + y, centerY - x, depth));
        plotTransformedPoint(colorBuffer, color, ScreenVertex(centerX - y, centerY - x, depth));
    };

    auto plotCircleFilling = [&](int x, int y) {
        float z = depth;  // Using constant depth for flat circle

        // Horizontal lines (y-offset)
        plotLineBoundsChecking(
            colorBuffer, color,
            centerY + y,  // upper line
            centerX - x, centerX + x,
            z, z, width
        );
        
        plotLineBoundsChecking(
            colorBuffer, color,
            centerY - y,  // lower line
            centerX - x, centerX + x,
            z, z, width
        );

        // Vertical lines (x-offset)
        plotLineBoundsChecking(
            colorBuffer, color,
            centerY + x,  // right line
            centerX - y, centerX + y,
            z, z, width
        );
        
        plotLineBoundsChecking(
            colorBuffer, color,
            centerY - x,  // left line
            centerX - y, centerX + y,
            z, z, width
        );
    };

    std::function<void(int, int)> plotFunction;
    if (context.currentAreaMode == SGL_LINE) {
        plotFunction = plotCirclePoints;
    }
    else {
        plotFunction = plotCircleFilling;
    }

    while (y >= x) {
        plotFunction(x, y);
        if (d < 0) {
            d += 4 * x + 6;
        } else {
            y--;
            d += 4 * (x - y) + 10;
        }
        x++;
    }
}


void initFillingStruct(FillingStruct& filler, ScreenVertex c1, ScreenVertex c2, int *maxX, int *minX) {
    // remove horizontal edges
    if (c1.y == c2.y) {
        return;
    }
    
    ScreenVertex top = (c1.y > c2.y) ? c1 : c2;
    ScreenVertex bottom = (c1.y > c2.y) ? c2 : c1;

    Edge edge = Edge(top, bottom);
    // edge shortening
    edge.bottomY++;
    filler.edges.push_back(edge);

    // find extremes
    if (top.y > filler.maxY) {
        filler.maxY = top.y;
    }
    if (bottom.y < filler.minY) {
        filler.minY = bottom.y;
    }
    if (top.x > *maxX || bottom.x > *maxX) {
        *maxX = max(top.x, bottom.x);
    }
    if (top.x < *minX || bottom.x < *minX) {
        *minX = min(top.x, bottom.x);
    }
}

void shakeSort(vector<Edge>& edges) {
    bool swapped = true;
    int start = 0;
    int end = edges.size() - 1;

    while (swapped) {
        swapped = false;

        for (int i = start; i < end; ++i) {
            if (edges[i].currentX > edges[i + 1].currentX) {
                std::swap(edges[i], edges[i + 1]);
                swapped = true;
            }
        }

        if (!swapped) {
            break;
        }
        swapped = false;

        --end;

        for (int i = end - 1; i >= start; --i) {
            if (edges[i].currentX > edges[i + 1].currentX) {
                std::swap(edges[i], edges[i + 1]);
                swapped = true;
            }
        }
        ++start;
    }
}

void updateActiveEdgeList(FillingStruct& filler, int y) {
    // remove already processed edges
    filler.activeEdgeList.erase(
        std::remove_if(filler.activeEdgeList.begin(), filler.activeEdgeList.end(),
            [y](const Edge& e) { return y < e.bottomY; }),
        filler.activeEdgeList.end());

    // add new intersecting edges
    vector<Edge> filtered;
    for (auto it = filler.edges.begin(); it != filler.edges.end();) {
        if (y <= it->topY) {
            it->currentX += (it->topY - y) * it->stepX;
            filler.activeEdgeList.push_back(*it);
            it = filler.edges.erase(it);  // Remove processed edge from filler.edges
        }
        else {
            ++it;
        }
    }
}

void fillPolygon() {
    auto& context = sceneManager->getCurrentContext();
    auto& colorBuffer = *(context.colorBuffer);
    auto& color = context.currentColor;
    auto& width = context.width;
    auto& screenVertices = *(context.screenVertices);
    unique_ptr<FillingStruct> filler = make_unique<FillingStruct>(context.height);
    int maxX = 0;
    int minX = width;

    // init phase
    for (size_t i = 0; i < screenVertices.size() - 1; i++) {
        initFillingStruct(*(filler), screenVertices[i], screenVertices[i + 1], &maxX, &minX);
    }
    initFillingStruct(*(filler), screenVertices.back(), screenVertices[0], &maxX, &minX);
    updateActiveEdgeList(*(filler), filler->maxY);
    // after init the edges are not partially sorted, so shake sort is not optimal
    sort(filler->activeEdgeList.begin(), filler->activeEdgeList.end(),
        [](const Edge& e1, const Edge& e2) { return e1.currentX < e2.currentX; });

    // clip scanning lines to canvas size
    filler->maxY = min(filler->maxY, context.height - 1);
    filler->minY = max(filler->minY, 0);

    // decide, whether to use ploting with bounds checking or without
    auto plotFunction = minX >= 0 && maxX < width ? 
        *(plotLine) : *(plotLineBoundsChecking);

    for (int y = filler->maxY; y > filler->minY; y--) {
        for (size_t i = 0; i < filler->activeEdgeList.size(); i+=2) {
            float z1 = filler->activeEdgeList[i].currentZ;
            float z2 = filler->activeEdgeList[i + 1].currentZ;

            // draw lines
            plotFunction(colorBuffer, color, y,
                round(filler->activeEdgeList[i].currentX),
                round(filler->activeEdgeList[i + 1].currentX), z1, z2, width);

            // update X coordinate
            filler->activeEdgeList[i].currentX += filler->activeEdgeList[i].stepX;
            filler->activeEdgeList[i].currentZ += filler->activeEdgeList[i].stepZ;
            filler->activeEdgeList[i + 1].currentX += filler->activeEdgeList[i + 1].stepX;
            filler->activeEdgeList[i + 1].currentZ += filler->activeEdgeList[i + 1].stepZ;
        }
        
        updateActiveEdgeList(*(filler), y - 1);
        // use shake sort
        shakeSort(filler->activeEdgeList);
    }
}
