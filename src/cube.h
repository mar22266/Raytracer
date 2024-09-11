#pragma once

#include <glm/glm.hpp>
#include "object.h"
#include "material.h"
#include "intersect.h"

class Cube : public Object
{
public:
    // Constructor del cubo con dimensiones no uniformes
    Cube(const glm::vec3 &minCorner, const glm::vec3 &dimensions, const Material &mat);

    // Método para calcular la intersección del rayo con el cubo
    Intersect rayIntersect(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection) const override;

private:
    glm::vec3 minCorner;  // La esquina inferior del cubo
    glm::vec3 dimensions; // Dimensiones del cubo (longitud de cada lado)
};
