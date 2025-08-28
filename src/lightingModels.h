#pragma once

#include "context.h"
#include "structures.h"
#include "scene.h"

/**
 * @file lightingModel.h
 * @brief Lighting models for ray tracing
 * 
 */


/**
 * @brief Computes the color based on the Phong lighting model computation
 */
Pixel lightingPhong(const PointLight& light, 
                    const Vertex& intersectionPoint, 
                    const Vertex& normal,
                    const Vertex& rayOrigin,
                    const Material& hitMaterial);

Pixel lightingCookTorrance();