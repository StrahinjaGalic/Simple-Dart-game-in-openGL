#pragma once
#include <GL/glew.h>
#include <string>

class Background {
public:
    Background(const std::string& texturePath); // Constructor with the texture path
    ~Background();
    void render(); // Render the background

private:
    GLuint VAO, VBO, textureID, shaderProgram; // OpenGL objects
    void loadTexture(const std::string& texturePath); // Load texture from file
};

