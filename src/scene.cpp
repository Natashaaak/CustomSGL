#include "scene.h"

using std::make_unique;

float Ray::ComputeT(Vertex point){ 
	if(direction.x != 0){
		return (point.x - center.x) / direction.x; 
	}
    if (direction.y != 0){
		return (point.y - center.y) / direction.y;
    }
	if (direction.z != 0) {
		return (point.z - center.z) / direction.z;
    } 
    return 0;   
}

Scene::Scene() {
	lightsList = make_unique<vector<PointLight>>();
	materialsList = make_unique<vector<Material>>();
}

void Scene::RestartScene() {
	primitivesList.clear();
	lightsList->clear();
	materialsList->clear();
}

Vertex Triangle::ComputeNormal(const Vertex& point) {
    Vertex v1 = points[1] - points[0];
    Vertex v2 = points[2] - points[0];
    Vertex normal = CrossProd(v1, v2);
    normal.Normalize();
    return normal;
}

Vertex Sphere::ComputeNormal(const Vertex& point) {
    Vertex normal = point - center;
    normal.Normalize();
    return normal;
}

Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3) {
	points = { v1, v2, v3 };
};

// Find intersection point - from PBRT - www.pbrt.org
bool Triangle::IntersectWithRay(const Ray& ray, float& tHit) {
	// Get triangle vertices in _p1_, _p2_, and _p3_
	const Vertex& p1 = points[0];
	const Vertex& p2 = points[1];
	const Vertex& p3 = points[2];
	Vertex e1 = p2 - p1;
	Vertex e2 = p3 - p1;
	Vertex s1 = CrossProd(ray.direction, e2);
	float divisor = DotProd(s1, e1);
	if (divisor == 0.) {
		return false;
	}
	float invDivisor = 1.f / divisor;
	// Compute first barycentric coordinate
	Vertex d = ray.center - p1;
	float b1 = DotProd(d, s1) * invDivisor;
	if (b1 < 0. || b1 > 1.) {
		return false;
	}
	// Compute second barycentric coordinate
	Vertex s2 = CrossProd(d, e1);
	float b2 = DotProd(ray.direction, s2) * invDivisor;
	if (b2 < 0. || (b1 + b2) > 1.) {
		return false;
	}
	// Compute _t_ to intersection point
	tHit = DotProd(e2, s2) * invDivisor;
	return (tHit > EPSILON_T);
}


// source: http://www.devmaster.net/wiki/Ray-sphere_intersection
bool Sphere::IntersectWithRay(const Ray& ray, float& t) {
	const Vertex dst = ray.center - center;
	const float b = DotProd(dst, ray.direction);
	const float c = DotProd(dst, dst) - radius * radius;
	const float d = b * b - c;  // Discriminant

	if (d < 0.0f) {
		return false;  // No intersection
	}

	// Compute both potential intersection points
	float t1 = -b - sqrtf(d);
	float t2 = -b + sqrtf(d);

	// Ensure t1 is the closer intersection
	if (t1 > t2) {
		std::swap(t1, t2);
	}

	// Check if the intersections are valid
	if (t1 >= EPSILON_T) {
		t = t1;  // Closest valid intersection
		return true;
	}
	else if (t2 >= EPSILON_T) {
		t = t2;  // Second intersection is valid
		return true;
	}

	return false;  // Both intersections are behind the ray origin
}
