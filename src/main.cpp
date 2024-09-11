#define FOV glm::radians(90.0f)
#define ASPECT_RATIO (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT
#define BIAS 0.01f
#define MAX_RECURSION_DEPTH 2

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <execution>

#include "./headers/skybox.h"
#include "./headers/light.h"
#include "./headers/color.h"
#include "./headers/object.h"
#include "./headers/cube.h"
#include "./headers/camera.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

SDL_Renderer *renderer = nullptr;
Light light(glm::vec3(20.0f, 0.0f, 0.0f), 1.5f, Color(255, 255, 255));
Camera camera(glm::vec3(-20.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 10.0f);
Skybox skybox("./textures/sky.jpg");

float castShadow(const glm::vec3 &shadowOrig, const glm::vec3 &lightDir, const std::vector<Object *> &objects, Object *hitObject)
{
    for (auto &obj : objects)
    {
        if (obj != hitObject)
        {
            Intersect shadowIntersect = obj->rayIntersect(shadowOrig, lightDir);
            if (shadowIntersect.isIntersecting && shadowIntersect.distance > 0)
            {
                const float shadowIntensity = (1.0f - glm::min(1.0f, shadowIntersect.distance / glm::length(light.position - shadowOrig)));
                return shadowIntensity;
            }
        }
    }
    return 1.0f;
}

Color castRay(const glm::vec3 &orig, const glm::vec3 &dir, const std::vector<Object *> &objects, float deltaTime, const short recursion = 0)
{
    Intersect intersect;
    Object *hitObject = nullptr;
    float zBuffer = std::numeric_limits<float>::infinity();

    for (auto &obj : objects)
    {
        Intersect tempIntersect = obj->rayIntersect(orig, dir);
        if (tempIntersect.isIntersecting && tempIntersect.distance < zBuffer)
        {
            zBuffer = tempIntersect.distance;
            intersect = tempIntersect;
            hitObject = obj;
        }
    }

    if (!intersect.isIntersecting || recursion >= MAX_RECURSION_DEPTH)
    {
        return skybox.getColor(dir); // Sky color
    }

    const Material &hitMaterial = hitObject->getMaterial();
    glm::vec3 lightDir = glm::normalize(light.position - intersect.point);
    glm::vec3 viewDir = glm::normalize(orig - intersect.point);

    float shadowIntensity = castShadow(intersect.point + BIAS * intersect.normal, lightDir, objects, hitObject);
    float intensity = shadowIntensity * light.intensity;

    float diffuseLightIntensity = std::max(0.0f, glm::dot(intersect.normal, lightDir));
    glm::vec3 reflectDir = glm::reflect(-lightDir, intersect.normal);
    float spec = std::pow(std::max(0.0f, glm::dot(viewDir, reflectDir)), hitMaterial.specularCoefficient);

    // Usar el método GetDiffuse para manejar texturas animadas
    Color diffuseLight = intensity * diffuseLightIntensity * hitMaterial.albedo * hitMaterial.GetDiffuse(deltaTime);
    Color specularLight = intensity * spec * hitMaterial.specularAlbedo * light.color;

    Color reflectedColor(0.0f, 0.0f, 0.0f);
    if (hitMaterial.reflectivity > 0)
    {
        glm::vec3 offsetOrigin = intersect.point + intersect.normal * BIAS;
        reflectedColor = hitMaterial.reflectivity * castRay(offsetOrigin, reflectDir, objects, deltaTime, recursion + 1);
    }

    Color refractedColor(0.0f, 0.0f, 0.0f);
    if (hitMaterial.transparency > 0)
    {
        glm::vec3 refractDir = glm::refract(dir, intersect.normal, hitMaterial.refractionIndex);
        glm::vec3 offsetOrigin = intersect.point - intersect.normal * BIAS;
        refractedColor = hitMaterial.transparency * castRay(offsetOrigin, refractDir, objects, deltaTime, recursion + 1);
    }

    return (1 - hitMaterial.reflectivity - hitMaterial.transparency) * (diffuseLight + specularLight) + reflectedColor + refractedColor;
}

void pixel(glm::vec2 position, Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, position.x, position.y);
}

void renderFromBuffer(const std::array<std::array<Color, SCREEN_WIDTH>, SCREEN_HEIGHT> &pixels)
{
    int textureWidth = SCREEN_WIDTH;
    int textureHeight = SCREEN_HEIGHT;

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);

    void *texturePixels;
    int pitch;

    SDL_LockTexture(texture, NULL, &texturePixels, &pitch);

    Uint32 format = SDL_PIXELFORMAT_ARGB8888;
    SDL_PixelFormat *mappingFormat = SDL_AllocFormat(format);

    Uint32 *texturePixels32 = static_cast<Uint32 *>(texturePixels);
    for (int y = 0; y < textureHeight; y++)
    {
        for (int x = 0; x < textureWidth; x++)
        {
            int index = y * (pitch / sizeof(Uint32)) + x;
            const Color &color = pixels[y][x];
            texturePixels32[index] = SDL_MapRGBA(mappingFormat, color.r, color.g, color.b, color.a);
        }
    }

    SDL_UnlockTexture(texture);
    SDL_Rect textureRect = {0, 0, textureWidth, textureHeight};
    SDL_RenderCopy(renderer, texture, NULL, &textureRect);
    SDL_DestroyTexture(texture);
}

void render(std::vector<Object *> &objects, float deltaTime)
{
    glm::vec3 simulatedUp = glm::vec3(0, 1, 0);
    std::vector<int> rows(SCREEN_HEIGHT);
    std::iota(rows.begin(), rows.end(), 0);
    std::array<std::array<Color, SCREEN_WIDTH>, SCREEN_HEIGHT> pixels;

    std::for_each(std::execution::par, rows.begin(), rows.end(), [&](int y)
                  {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            float screenX =  (2.0f * x) / SCREEN_WIDTH - 1.0f;
            float screenY = -(2.0f * y) / SCREEN_HEIGHT + 1.0f;

            screenX *= ASPECT_RATIO;

            glm::vec3 dir = glm::normalize(camera.target - camera.position);
            glm::vec3 right = glm::normalize(glm::cross(dir, simulatedUp));
            glm::vec3 up = glm::cross(right, dir);

            dir = dir + right * screenX + up * screenY;

            Color pixelColor = castRay(camera.position, glm::normalize(dir), objects, deltaTime);

            pixels[y][x] = pixelColor;
        } });

    renderFromBuffer(pixels);
}

void handleKeyPress(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_UP:
        camera.move(-1.0f);
        break;
    case SDLK_DOWN:
        camera.move(1.0f);
        break;
    case SDLK_a:
        camera.rotate(-1.0f, 0.0f);
        break;
    case SDLK_d:
        camera.rotate(1.0f, 0.0f);
        break;
    case SDLK_w:
        camera.rotate(0.0f, -1.0f);
        break;
    case SDLK_s:
        camera.rotate(0.0f, 1.0f);
        break;
    default:
        break;
    }
}

void processKeyEvents(const SDL_Event &event, std::unordered_map<SDL_Keycode, bool> &keyStates)
{
    switch (event.type)
    {
    case SDL_KEYDOWN:
        handleKeyPress(event.key.keysym.sym);
        keyStates[event.key.keysym.sym] = true;
        break;
    case SDL_KEYUP:
        keyStates[event.key.keysym.sym] = false;
        break;
    default:
        break;
    }
}

// Function to load water animation frames with enhanced vibrant blue shades
std::vector<Color> loadWaterFrames()
{
    std::vector<Color> frames;
    // Add more distinct and vibrant blue shades to make the water appear more animated and visible
    frames.push_back(Color(0, 162, 255));  // Bright sky blue
    frames.push_back(Color(0, 102, 255));  // More saturated medium blue
    frames.push_back(Color(0, 51, 204));   // Deep vibrant blue
    frames.push_back(Color(0, 76, 230));   // Light but vibrant blue
    frames.push_back(Color(0, 0, 255));    // Pure blue
    frames.push_back(Color(30, 144, 255)); // Dodger blue
    frames.push_back(Color(0, 191, 255));  // Deep sky blue
    return frames;
}

int main(int argc, char *args[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "Ray Tracer",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool isRunning = true;
    SDL_Event event;
    float rotateAngle = 0.0f;

    unsigned int lastTime = SDL_GetTicks();
    unsigned int currentTime;
    float dT;

    Material ivory(
        Color(100, 100, 80),
        0.6f,
        0.3f,
        50.0f,
        0.2f);
    Material rubber(
        Color(80, 0, 0),
        0.9f,
        0.1f,
        10.0f,
        0.0f);

    Material emerald(
        Color(80, 200, 120), // Green color typical of emerald
        0.2f,                // Low albedo
        0.95f,               // Very high specular albedo
        76.0f,               // High specular coefficient for a shiny surface
        0.2f,                // Low reflectivity
        0.7f,                // High transparency
        1.57f);              // Refraction index of emerald

    Material glass(
        Color(255, 255, 255),
        0.1f,
        1.0f,
        125.0f,
        0.0f,
        0.9f,
        0.1f);

    Material ice(
        Color(173, 216, 230), // Light blue color for ice
        0.1f,                 // Low albedo
        0.5f,                 // Medium specular albedo
        25.0f,                // Specular coefficient for a slight shiny surface
        0.1f,                 // Low reflectivity
        0.8f,                 // High transparency
        1.31f);               // Refraction index similar to real ice

    Material wood(
        Color(139, 69, 19), // Brown color for wood
        0.5f,               // Medium albedo
        0.2f,               // Low specular albedo
        20.0f,              // Specular coefficient for a slightly glossy surface
        0.1f,               // Low reflectivity
        0.0f,               // No transparency
        0.0f);              // No refraction index needed

    Material water(
        Color(28, 134, 238), // Blue color for water
        0.1f,                // Low albedo
        0.6f,                // Medium specular albedo
        50.0f,               // Specular coefficient for a reflective surface
        0.5f,                // Medium reflectivity
        0.9f,                // High transparency
        1.33f);              // Refraction index close to real water

    Material gold(
        Color(255, 230, 0), // Brighter yellow color for gold
        0.8f,               // Increased albedo for more brightness
        0.98f,              // Even higher specular albedo for extra shine
        150.0f,             // Increased specular coefficient for enhanced shininess
        0.9f,               // Higher reflectivity to make it more reflective
        0.0f,               // No transparency
        0.0f);              // No refraction index needed

    Material velvet(
        Color(204, 0, 204), // Rich purple color for velvet
        0.8f,               // High albedo
        0.1f,               // Low specular albedo
        10.0f,              // Low specular coefficient for a soft, matte finish
        0.01f,              // Very low reflectivity
        0.0f,               // No transparency
        0.0f);              // No refraction index needed
    Material leather(
        Color(105, 55, 5), // Dark brown color for leather
        0.4f,              // Medium albedo
        0.25f,             // Low-medium specular albedo
        22.0f,             // Specular coefficient for a mild sheen
        0.05f,             // Very low reflectivity
        0.0f,              // No transparency
        0.0f);             // No refraction index needed
    Material leaves(
        Color(34, 139, 34), // Green color for leaves
        0.2f,               // Low albedo to simulate the light absorption by leaves
        0.5f,               // Medium specular albedo for a slight glossy surface
        10.0f,              // Lower specular coefficient for a soft sheen
        0.1f,               // Low reflectivity to mimic the light reflecting nature of leaves
        0.3f,               // Slight transparency to simulate light passing through leaves
        1.0f);              // Refraction index close to that of water, typical for organic materials

    // Load frames for the animated water
    std::vector<Color> waterFrames = loadWaterFrames();
    std::vector<Object *> objects;

    // Create the AnimatedTexture object for water with a frame rate of 5 frames per second
    std::shared_ptr<AnimatedTexture> animatedWaterTexture = std::make_shared<AnimatedTexture>(AnimatedTexture(waterFrames, 5.0f));

    // Create the Material using the animated texture for water
    // Create the Material using the animated texture for water with adjusted parameters for enhanced visibility
    // Create the Material using the animated texture for water with highly enhanced visibility parameters
    Material animatedWaterMaterial(animatedWaterTexture, 0.5f, 1.0f, 75.0f, 0.5f, 0.98f, 1.33f); // Highly enhanced parameters for maximum visibility and realism
                                                                                                 // Adjusted parameters for more visibility and realism
                                                                                                 // Customize parameters as needed

    // Ensure the Cube class has texture coordinates correctly set up as shown above.

    // Add the animated water cube to the scene with the animated material
    // Ensure the size and positioning are adjusted as needed for your scene
    objects.push_back(new Cube(glm::vec3(30, 0, 0), glm::vec3(5, 15, 3), animatedWaterMaterial));

    // Other objects in your scene...
    // Simple river across the ground, centered, using the animated water material
    objects.push_back(new Cube(glm::vec3(25, 0, 0), glm::vec3(30, 1, 30), animatedWaterMaterial));

    // Extended ground around the river to simulate riverbanks, on one side
    objects.push_back(new Cube(glm::vec3(0, 0, 0), glm::vec3(30, 2, 30), leather));

    // Extended ground on the other side of the river
    objects.push_back(new Cube(glm::vec3(35, 0, 0), glm::vec3(30, 2, 30), leather));

    // Trees of various sizes
    objects.push_back(new Cube(glm::vec3(5, 1, 5), glm::vec3(4, 12, 4), wood));   // Larger tree trunk
    objects.push_back(new Cube(glm::vec3(5, 13, 5), glm::vec3(8, 8, 8), leaves)); // Larger tree canopy

    objects.push_back(new Cube(glm::vec3(15, 1, 15), glm::vec3(4, 15, 4), wood));      // Another larger tree
    objects.push_back(new Cube(glm::vec3(15, 16, 15), glm::vec3(10, 10, 10), leaves)); // Another larger tree canopy

    // Adding two more trees to enhance the scene
    objects.push_back(new Cube(glm::vec3(45, 1, 10), glm::vec3(4, 12, 4), wood));   // New tree trunk
    objects.push_back(new Cube(glm::vec3(45, 13, 10), glm::vec3(8, 8, 8), velvet)); // New tree canopy

    objects.push_back(new Cube(glm::vec3(55, 1, 5), glm::vec3(4, 15, 4), wood));      // Another new tree trunk
    objects.push_back(new Cube(glm::vec3(55, 16, 5), glm::vec3(10, 10, 10), velvet)); // Another new tree canopy

    int frameCount = 0;
    float elapsedTime = 0.0f;

    std::unordered_map<SDL_Keycode, bool> keyStates;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                isRunning = false;
                break;
            default:
                processKeyEvents(event, keyStates);
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render(objects, dT);

        SDL_RenderPresent(renderer);

        // Calculate the deltaTime
        currentTime = SDL_GetTicks();
        dT = (currentTime - lastTime) / 1000.0f; // Time since last frame in seconds
        lastTime = currentTime;

        frameCount++;
        elapsedTime += dT;
        if (elapsedTime >= 1.0f)
        {
            float fps = static_cast<float>(frameCount) / elapsedTime;
            std::cout << "FPS: " << fps << std::endl;

            frameCount = 0;
            elapsedTime = 0.0f;
        }
    }

    for (Object *object : objects)
    {
        delete object;
    }
    objects.clear();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
