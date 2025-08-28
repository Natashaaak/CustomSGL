#pragma once

#include "context.h"    
#include "structures.h"
#include "scene.h"
#include "lightingModels.h"
#include "ray_tracing_utils.h"
#include <vector>
#include <limits>
#include <iostream>

using std::unique_ptr;
using std::make_unique;

const bool USE_ANTIALIASING = false;
const float ANTIALIASING_WEIGHT = 0.8f;
const float DIFFERENCE_EPSILON = 0.1f;
const int MAX_RECURSION_DEPTH = 8;

// Small offset to avoid self-intersection
const float INTERSECTION_BIAS = 0.0001f;

/**
 * @brief Casts a ray into the scene for a specific pixel and updates the color buffer.
 *
 * This function generates a primary ray based on the given pixel coordinates and the
 * inverse View-Projection-Matrix (invVPM). It traces the ray through the scene, determines
 * the color at the intersection, and stores it in the color buffer of the current rendering
 * context.
 *
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param w The width of the rendering surface (used for buffer indexing).
 * @param invVPM The inverse View-Projection-Matrix, used to transform screen coordinates
 *               into world space.
 */
void castRay(int x, int y, int w, const Matrix& invVPM);

/**
 * @brief Converts pixel coordinates to Normalized Device Coordinates (NDC).
 *
 * This function transforms pixel coordinates from screen space into Normalized Device
 * Coordinates (NDC) space, which ranges from -1 to 1 in both x and y dimensions.
 * The transformation considers the dimensions of the current rendering context.
 *
 * @param x The x-coordinate of the pixel in screen space.
 * @param y The y-coordinate of the pixel in screen space.
 * @return A Vertex representing the NDC coordinates, with z set to -1.0f and w set to 1.0f.
 */
Vertex pixelToNDCSpace(float x, float y);

/**
 * @brief Generates a primary ray for a given pixel in screen space.
 *
 * This function computes a primary ray by converting the pixel coordinates to Normalized Device
 * Coordinates (NDC), transforming the NDC near and far points into world space using the
 * inverse Projection-View-Matrix (invPVM), and calculating the ray's direction. The resulting
 * ray originates from the near point in world space and points towards the far point.
 *
 * @param x The x-coordinate of the pixel in screen space.
 * @param y The y-coordinate of the pixel in screen space.
 * @param invPVM The inverse Projection-View-Matrix, used to transform points from NDC to world space.
 * @return A Ray object, containing the origin (worldNear) and direction of the ray.
 */
Ray generatePrimaryRay(float x, float y, const Matrix& invPVM);

/**
 * @brief Checks whether a given point on a surface is visible from a light source.
 *
 * This function determines if the path between a surface intersection point and a light source
 * is obstructed by any primitive in the scene. It traces a shadow ray from the intersection point
 * toward the light source and checks for intersections with other objects.
 *
 * @param intersectionPoint The point of intersection on the surface.
 * @param light The light source to check visibility against.
 * @return true If the point is visible from the light source (no obstruction).
 * @return false If the point is not visible from the light source (obstructed by another primitive).
 *
 * @note This function uses an EPSILON value to avoid self-shadowing due to floating-point precision errors.
 */
bool checkVisibility(const Vertex& intersectionPoint, const PointLight& light);

/**
 * @brief Traces a ray through the scene and computes the resulting pixel color.
 *
 * This function traces a given ray to find its closest intersection with the scene's primitives.
 * If an intersection is found, it computes the color at the intersection point using a Phong
 * lighting model. If no intersection is found, the scene's clear color is returned.
 *
 * @param ray The ray to be traced through the scene. Contains origin and direction.
 * @return A Pixel object representing the computed color at the intersection or the clear color
 *         if no intersection occurs.
 */
Pixel traceRay(const Ray& ray, int depth);

/**
 * @brief Applies anti-aliasing to a pixel by sampling rays at sub-pixel positions and averaging the results.
 *
 * This function improves visual quality by reducing aliasing artifacts at a given pixel. It samples
 * additional rays at sub-pixel offsets, computes their colors, and combines them with the original pixel color
 * using a weighted average. The weight of the additional samples is defined by `ANTIALIASING_WEIGHT`.
 *
 * @param x The x-coordinate of the pixel.
 * @param y The y-coordinate of the pixel.
 * @param w The width of the rendering surface (used for buffer indexing).
 * @param invVPM The inverse View-Projection-Matrix, used to transform screen coordinates
 *               into world space for ray generation.
 */
void antialiaseRay(int x, int y, int w, const Matrix& invVPM);

/**
 * @brief Applies anti-aliasing to the entire scene by detecting edge pixels and refining their colors.
 *
 * This function scans the color buffer to detect pixels with significant color differences compared
 * to their neighbors, indicating potential edges. For such edge pixels, it invokes `antialiazeRay`
 * to apply sub-pixel sampling and improve visual smoothness. The process is performed on the interior
 * pixels of the rendering surface, leaving the edges untouched.
 *
 * @param invPVM The inverse Projection-View-Matrix, used for ray generation in sub-pixel sampling.
 */
void antialiase(const Matrix& invPVM);
