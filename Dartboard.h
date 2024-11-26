#pragma once
#include <vector>
#include <GL/glew.h>
#include <utility>  // For std::pair

class Dartboard {
public:
    Dartboard(const char* texturePath, const char* vertexShaderPath, const char* fragmentShaderPath);
    ~Dartboard();

    void render();
    int calculateScore(float x, float y);
    void recordHit(float x, float y);
    void renderHitMarkers();
    void clearHits();

private:
    unsigned int VAO, VBO;
    unsigned int shaderProgram;
    unsigned int textureID;
    unsigned int markerVAO, markerVBO;
    unsigned int markerShaderProgram;

    static const int NUM_SEGMENTS = 100;
    static const float RADIUS;

    std::vector<float> vertices;
    int sectors[20] = { 20, 5, 12, 9, 14, 11, 8, 16, 7, 19, 3, 17, 2, 15, 10, 6, 13, 4, 18, 1 };
    std::vector<std::pair<float, float>> hitPositions; // Store hit positions

    void generateCircleVertices();
    unsigned int loadImageToTexture(const char* filePath);
    unsigned int compileShader(GLenum type, const char* sourcePath);
    unsigned int createShader(const char* vertexShaderPath, const char* fragmentShaderPath);
    void setupMarker();              // Setup OpenGL for hit marker
};
