#pragma once
#include "context.h"
#include <iostream>

/**
 * @brief Draws points based on the current context.
 * 
 * This function iterates through the primitives list in the current context
 * and draws points with the specified size and color.
 */
void drawPoints();

/**
 * @brief Transforms a vertex using the current context's VPM matrix.
 * 
 * @param v The vertex to be transformed.
 * @return Vertex The transformed vertex.
 */
Vertex transformPoint(const Vertex& v);

/**
 * @brief Sets the scale factor based on the current context's VPM matrix.
 * 
 * This function calculates and sets the scale factor used for transformations.
 */
void setScaleFactor();

/**
 * @brief Determines the increment direction between two points.
 * 
 * @param start The starting point.
 * @param end The ending point.
 * @return int The increment value (-1, 0, or 1).
 */
int getIncrement(int start, int end);

/**
 * @brief Draws a line using Bresenham's line algorithm.
 * 
 * @param v1 The starting vertex of the line.
 * @param v2 The ending vertex of the line.
 */
void drawBresenhamLine(Vertex v1, Vertex v2);

/**
 * @brief Draws multiple lines based on the primitives list in the current context.
 * 
 * This function draws lines by connecting pairs of vertices in the primitives list.
 */
void drawLines();

/**
 * @brief Draws a line strip based on the primitives list in the current context.
 * 
 * This function draws connected lines by joining consecutive vertices in the primitives list.
 */
void drawLineStrip();

/**
 * @brief Draws a closed line loop based on the primitives list in the current context.
 * 
 * This function draws connected lines by joining consecutive vertices and closing the loop
 * by connecting the last vertex to the first one.
 */
void drawLineLoop();

/**
 * @brief Draws a circle using Bresenham's circle algorithm.
 * 
 * @param sX The x-coordinate of the circle's center.
 * @param sY The y-coordinate of the circle's center.
 * @param z The z-coordinate of the circle's center.
 * @param radius The radius of the circle.
 */
void drawBresenhamCircle(float sX, float sY, float z, float radius);

void fillPolygon();
