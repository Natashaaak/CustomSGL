#include "ray_tracing_utils.h"

Vertex pixelToNDCSpace(float x, float y) {
    int width  = sceneManager->getCurrentContext().width;
    int height = sceneManager->getCurrentContext().height;
    float ndcX = (2.0f * x) / width - 1.0f;
    float ndcY = -1.0f + (2.0f * y) / height;
    return Vertex(ndcX, ndcY, -1.0f, 1.0f);
}

Ray generatePrimaryRay(float x, float y, const Matrix& invPVM) {
    // Convert pixel to NDC without applying viewport matrix
    Vertex ndcPoint = pixelToNDCSpace(x, y);
    // Create near and far points in NDC
    Vertex nearPoint = Vertex(ndcPoint.x, ndcPoint.y, -1.0f, 1.0f);
    Vertex farPoint = Vertex(ndcPoint.x, ndcPoint.y, 1.0f, 1.0f);
    
    // Transform to world space
    Vertex worldNear = invPVM * nearPoint;
    Vertex worldFar = invPVM * farPoint;
    
    // Perspective divide
    worldNear /= worldNear.w;
    worldFar /= worldFar.w;
    
    // Calculate ray direction
    Vertex rayDirection = worldFar - worldNear;
    rayDirection.Normalize();
    
    return Ray(worldNear, rayDirection);
}

bool checkVisibility(const Vertex& intersectionPoint, const PointLight& light) {
    Scene& scene = sceneManager->getCurrentContext().scene;

    Vertex lightDir = light.center - intersectionPoint;
    lightDir.Normalize();
    Ray shadowRay = Ray(intersectionPoint, lightDir);
    float lightHit = shadowRay.ComputeT(light.center) - EPSILON_T;
    for (const auto& primitive : scene.primitivesList) {
        float tHit = 0.0f;
        if (primitive->IntersectWithRay(shadowRay, tHit) && tHit < lightHit) {
            return false;
        }
    }
    return true;
}

// TODO: add doxygen
Primitive3D* FindClosestIntersection(const Scene& scene, const Ray& ray, float& closestT) {
    closestT = std::numeric_limits<float>::infinity();
    Primitive3D* closestPrimitive = nullptr;
    for (const auto& primitive : scene.primitivesList) {
        float tHit = 0.0f;

        if (primitive->IntersectWithRay(ray, tHit) && tHit < closestT) {
            // check for back face culling
            Vertex normal = primitive->ComputeNormal(ray.center + ray.direction * tHit);
            float dotProduct = DotProd(normal, ray.direction);
            
            Material mat = scene.materialsList->at(primitive->materialID);
            
            // Skip backface culling for transparent objects
            if (mat.T <= 0 && dotProduct > 0) {
                continue;
            }
            
            closestT = tHit;
            closestPrimitive = primitive.get();
        }
    }

    return closestPrimitive;
}

// TODO: add doxygen
bool RefractRay(Vertex normal, float IOR, const Ray& originalRay, Ray& refractedRay) {
    float gamma, sqrterm;
    float dot = DotProd(originalRay.direction, normal);
    Vertex dir;

    if (dot < 0.0) {
        // from outside into the inside of object
        gamma = 1.0 / IOR;
    }
    else {
        // from the inside to outside of object
        gamma = IOR;
        dot = -dot;
        normal = normal * -1;
    }
    sqrterm = 1.0 - (gamma * gamma * (1.0 - dot * dot));

    // Check for total internal reflection, do nothing if it applies.
    if (sqrterm > 0.0) {
        sqrterm = (dot * gamma) + sqrt(sqrterm);
        dir = (normal * -sqrterm) + (originalRay.direction * gamma);
        refractedRay = Ray(originalRay.center, dir);
        return true;
    }

    return false;
}

Pixel traceRay(const Ray& ray, int depth) {
    Scene& scene = sceneManager->getCurrentContext().scene;
    // Closest intersection point
    float closestT;
    Primitive3D* closestPrimitive = FindClosestIntersection(scene, ray, closestT);

    if (!closestPrimitive) {
        // take color from environment map
        if (scene.envMap) {
            float c = sqrt(ray.direction.x * ray.direction.x + ray.direction.y * ray.direction.y);
            float r = c > 0.f ? acos(ray.direction.z) / (2 * c * M_PI) : 0.f;
            int u = (0.5f + r * ray.direction.x) * scene.envMap->width;
            int v = (0.5f - r * ray.direction.y) * scene.envMap->height;
            int id = 3 * (u + v * scene.envMap->width);
            return Pixel(scene.envMap->texels[id], scene.envMap->texels[id + 1], scene.envMap->texels[id + 2]);
        }
        return sceneManager->getCurrentContext().clearColor;
    }

    Vertex intersectionPoint = ray.center + (ray.direction * closestT);
    Pixel color = Pixel(0.0f, 0.0f, 0.0f);
    Vertex normal = closestPrimitive->ComputeNormal(intersectionPoint);
    Material mat = scene.materialsList->at(closestPrimitive->materialID);
    
    const Vertex biasedPoint = intersectionPoint + normal * INTERSECTION_BIAS;

    // Lighting model computation
    for (const PointLight& light : *scene.lightsList) {
        // cast shadow rays
        if (checkVisibility(biasedPoint, light)) {
            color += lightingPhong(light, intersectionPoint, normal, ray.center, mat);
        }
    }

    // reflected + refracted ray
    if (depth < MAX_RECURSION_DEPTH) {
        // reflected
        if(mat.KSpecular > 0){
            Vertex reflectedDir = ray.direction - normal * (2 * DotProd(normal, ray.direction));
            reflectedDir.Normalize();
            // Use biased point for reflection ray origin
            Ray reflectedRay(biasedPoint, reflectedDir);
            Pixel reflectedColor = traceRay(reflectedRay, depth + 1);
            color += reflectedColor * mat.KSpecular;
        }
        
        // refracted
        if (mat.T > 0) {
            Ray refracted = ray;
            if (RefractRay(normal, mat.IOR, ray, refracted)) {
                refracted.direction.Normalize();
                // Use negative bias for refraction ray origin (going into the object)
                Vertex refractedPoint = intersectionPoint - normal * INTERSECTION_BIAS;
                Ray refractedRay(refractedPoint, refracted.direction);
                color += traceRay(refractedRay, depth + 1) * mat.T;
            }
        }
    }

    return color;
}

inline bool checkDifference(const Pixel& origin, const Pixel& neighbour) {
    if (abs(origin.r - neighbour.r) <= DIFFERENCE_EPSILON &&
        abs(origin.g - neighbour.g) <= DIFFERENCE_EPSILON &&
        abs(origin.b - neighbour.b) <= DIFFERENCE_EPSILON) {
            return false;
        }
    return true;
}

void antialiaseRay(int x, int y, int w, const Matrix& invVPM) {
    auto& currentContext = sceneManager->getCurrentContext();
    currentContext.colorBuffer->at(x + y * w) = currentContext.colorBuffer->at(x + y * w) * (1 - ANTIALIASING_WEIGHT);
    float weight = ANTIALIASING_WEIGHT / 4;

    for (int i = 1; i < 3; i++)
    {
        for (int j = 1; j < 3; j++)
        {
            Ray ray = generatePrimaryRay(x + 0.25f * j, y + 0.25f * i, invVPM);
            Pixel color = traceRay(ray, 0);
            currentContext.colorBuffer->at(x + y * w) += color * weight;
        }
    }
}

void antialiase(const Matrix& invPVM) {
    auto& context = sceneManager->getCurrentContext();
    auto& colorBuffer = *(context.colorBuffer);
    int width = context.width;
    int height = context.height;
    Pixel* origin = &colorBuffer[1];

    // top border
    for (int x = 1; x < width - 1; x++, origin++) {
        if (checkDifference(*origin, colorBuffer[x + 1]) ||
            checkDifference(*origin, colorBuffer[x - 1]) ||
            checkDifference(*origin, colorBuffer[x + width])) {
            antialiaseRay(x, 0, width, invPVM);
        }
    }

    for (int y = 1; y < height - 1; y++, origin++) {
        origin = &colorBuffer[y * width];
        // left border
        if (checkDifference(*origin, colorBuffer[(y + 1) * width]) ||
            checkDifference(*origin, colorBuffer[1 + y * width]) ||
            checkDifference(*origin, colorBuffer[(y - 1) * width])) {
            antialiaseRay(0, y, width, invPVM);
        }

        for (int x = 1; x < width - 1; x++, origin++) {
            if (checkDifference(*origin, colorBuffer[x + (y + 1) * width]) ||
                checkDifference(*origin, colorBuffer[x + 1 + y * width]) ||
                checkDifference(*origin, colorBuffer[x - 1 + y * width]) ||
                checkDifference(*origin, colorBuffer[x + (y - 1) * width])) {
                antialiaseRay(x, y, width, invPVM);
            }
        }
        // right border
        if (checkDifference(*origin, colorBuffer[(y + 1) * width]) ||
            checkDifference(*origin, colorBuffer[-1 + y * width]) ||
            checkDifference(*origin, colorBuffer[(y - 1) * width])) {
            antialiaseRay(width - 1, y, width, invPVM);
        }
    }
    // bottom border
    origin = &colorBuffer[1 + (height - 1) * width];
    for (int x = 1; x < width - 1; x++, origin++) {
        if (checkDifference(*origin, colorBuffer[x + 1 + (height - 1) * width]) ||
            checkDifference(*origin, colorBuffer[x - 1 + (height - 1) * width]) ||
            checkDifference(*origin, colorBuffer[x     + (height - 2) * width])) {
            antialiaseRay(x, height - 1, width, invPVM);
        }
    }
}
