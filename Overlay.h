#ifndef OVERLAY_H
#define OVERLAY_H

#include <GL/glew.h>
#include "Button.h"

class Overlay {
public:
    // Constructor and Destructor
    Overlay(const char* vertexShaderPath, const char* fragmentShaderPath, const char* quitButtonVertexPath, const char* quitButtonFragmentPath);
    ~Overlay();

    void renderPauseMenu();

private:
    // Shader program for overlay
    unsigned int shaderProgram;

    // VAO and VBO for geometry (screen quad)
    unsigned int VAO, VBO;

    Button resumeButton;  // Add a Button as a member of the Overlay class

    // Setup geometry for overlay rendering (quad)
    void setupOverlayGeometry();

    // Compile a shader from source file
    unsigned int compileShader(GLenum type, const char* sourcePath);

    // Create shader program using vertex and fragment shaders
    unsigned int createShader(const char* vertexShaderPath, const char* fragmentShaderPath);

    // Render a full-screen textured quad (helper)
    void renderTexturedQuad(float x, float y, float width, float height);


};

#endif // OVERLAY_H
