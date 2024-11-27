#ifndef BUTTON_H
#define BUTTON_H

#include <GL/glew.h>
#include <string>

class Button {
public:
    Button(float x, float y, float width, float height);
    ~Button();

    void render();
    void setShader(unsigned int shaderProgram);

private:
    float x, y, width, height;
    unsigned int shaderProgram; // Shader program for the button
    unsigned int VAO, VBO, EBO;       // OpenGL objects for button rendering

    // Setup geometry for button rendering
    void setupButtonGeometry();

    // Compile shader from source file
    unsigned int compileShader(GLenum type, const char* sourcePath);

    // Create shader program using vertex and fragment shaders
    unsigned int createShader(const char* vertexShaderPath, const char* fragmentShaderPath);
};

#endif // BUTTON_H

