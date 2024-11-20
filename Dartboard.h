#pragma once
#include <vector>
#include <GL/glew.h>

class Dartboard {
public:
    Dartboard(const char* texturePath, const char* vertexShaderPath, const char* fragmentShaderPath);
    ~Dartboard();

    void render();

private:
    unsigned int VAO, VBO;
    unsigned int shaderProgram;
    unsigned int textureID;

    static const int NUM_SEGMENTS = 100;
    static const float RADIUS;

    std::vector<float> vertices;

    void generateCircleVertices();
    unsigned int loadImageToTexture(const char* filePath);
    unsigned int compileShader(GLenum type, const char* sourcePath);
    unsigned int createShader(const char* vertexShaderPath, const char* fragmentShaderPath);
};


