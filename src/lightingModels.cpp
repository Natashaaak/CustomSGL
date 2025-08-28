#include "lightingModels.h"

Pixel lightingPhong(const PointLight& light, 
                    const Vertex& intersectionPoint, 
                    const Vertex& normal,
                    const Vertex& rayOrigin,
                    const Material& hitMaterial) {
    // directions
    Vertex lightDir = (light.center - intersectionPoint);
    lightDir.Normalize();
    Vertex viewDir = (rayOrigin - intersectionPoint);
    viewDir.Normalize();
    Vertex reflectedDir = normal * (2 * DotProd(normal, lightDir)) - lightDir;

    //  diffuse component
    float cosAlpha = std::max(0.0f, DotProd(normal, lightDir));
    Pixel diffuseColor = light.color * (hitMaterial.color * (hitMaterial.KDiffuse * cosAlpha));


    // specular component
    float cosBeta = std::max(0.0f, DotProd(reflectedDir, viewDir));
    float specular = std::pow(cosBeta, hitMaterial.shininess);
    Pixel specularColor = light.color * (hitMaterial.KSpecular * specular);

    // final color
    return diffuseColor + specularColor;
}