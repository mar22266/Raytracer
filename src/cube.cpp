#include "./headers/cube.h"

Cube::Cube(const glm::vec3 &minCorner, const glm::vec3 &dimensions, const Material &mat)
    : Object(mat), minCorner(minCorner), dimensions(dimensions) {}

// Método para calcular la intersección del rayo con el cubo
Intersect Cube::rayIntersect(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection) const
{
    glm::vec3 invDir = 1.0f / rayDirection;
    glm::vec3 t0 = (minCorner - rayOrigin) * invDir;
    glm::vec3 t1 = (minCorner + dimensions - rayOrigin) * invDir;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float tNear = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
    float tFar = glm::min(tmax.x, glm::min(tmax.y, tmax.z));

    if (tNear > tFar || tFar < 0)
    {
        return Intersect(); // No hay intersección
    }

    glm::vec3 hitPoint = rayOrigin + tNear * rayDirection;
    glm::vec3 normal = glm::vec3(0);

    // Determinar la normal dependiendo del lado del cubo que se intersecta
    if (tNear == tmin.x)
        normal = glm::vec3(-1, 0, 0);
    else if (tNear == tmax.x)
        normal = glm::vec3(1, 0, 0);
    else if (tNear == tmin.y)
        normal = glm::vec3(0, -1, 0);
    else if (tNear == tmax.y)
        normal = glm::vec3(0, 1, 0);
    else if (tNear == tmin.z)
        normal = glm::vec3(0, 0, -1);
    else if (tNear == tmax.z)
        normal = glm::vec3(0, 0, 1);

    return Intersect(hitPoint, normal, tNear);
}
