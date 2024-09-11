#pragma once

#include "color.h"
#include <vector>
#include <memory>

// Clase para manejar texturas animadas
class AnimatedTexture
{
public:
  std::vector<Color> frames; // Vector de colores que representan los frames de la animación (puede cambiarse a una clase de Textura si existe)
  float frameRate;           // Velocidad de la animación (frames por segundo)
  int currentFrame;          // Frame actual de la animación
  float timeAccumulator;     // Acumulador de tiempo para cambiar frames

  AnimatedTexture(const std::vector<Color> &frames, float frameRate)
      : frames(frames), frameRate(frameRate), currentFrame(0), timeAccumulator(0.0f) {}

  // Actualiza el frame actual basado en el tiempo transcurrido
  void Update(float deltaTime)
  {
    timeAccumulator += deltaTime;
    if (timeAccumulator >= 1.0f / frameRate)
    {
      currentFrame = (currentFrame + 1) % frames.size();
      timeAccumulator = 0.0f;
    }
  }

  // Obtiene el frame actual de la animación
  Color GetCurrentFrame() const
  {
    return frames[currentFrame];
  }
};

// Estructura de Material con soporte para texturas animadas
struct Material
{
  Color diffuse;
  float albedo;
  float specularAlbedo;
  float specularCoefficient; // The specular coefficient
  float reflectivity;        // The reflectivity of the material
  float transparency;        // The transparency of the material
  float refractionIndex;
  std::shared_ptr<AnimatedTexture> animatedTexture; // Puntero a la textura animada

  // Constructor para materiales sin textura animada
  Material(const Color &color, float albedo, float specularAlbedo, float specCoef, float reflectivity = 0, float transparency = 0, float refractionIndex = 0)
      : diffuse(color),
        albedo(albedo),
        specularAlbedo(specularAlbedo),
        specularCoefficient(specCoef),
        reflectivity(reflectivity),
        transparency(transparency),
        refractionIndex(refractionIndex),
        animatedTexture(nullptr) {}

  // Constructor para materiales con textura animada
  Material(const std::shared_ptr<AnimatedTexture> &animTexture, float albedo, float specularAlbedo, float specCoef, float reflectivity = 0, float transparency = 0, float refractionIndex = 0)
      : diffuse(Color(0, 0, 0)), // Se inicializa en negro o en un color por defecto
        albedo(albedo),
        specularAlbedo(specularAlbedo),
        specularCoefficient(specCoef),
        reflectivity(reflectivity),
        transparency(transparency),
        refractionIndex(refractionIndex),
        animatedTexture(animTexture)
  {
  }

  // Obtiene el color difuso del material, considerando la textura animada si existe
  Color GetDiffuse(float deltaTime) const
  {
    if (animatedTexture)
    {
      animatedTexture->Update(deltaTime);
      return animatedTexture->GetCurrentFrame();
    }
    return diffuse;
  }
};
