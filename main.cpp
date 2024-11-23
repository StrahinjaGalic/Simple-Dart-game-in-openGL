#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Dartboard.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include "Crosshair.h"
#include "Player.h"
#include "Background.h"

void processInput(GLFWwindow* window);
void updateGame(float deltaTime, GLFWwindow* window, Dartboard& dartboard);
void processThrow(Player& currentPlayer, Dartboard& dartboard);
void checkOpenGLError(const char* description);

// Game objects
Player player1("Player 1");
Player player2("Player 2");
Crosshair crosshair;

int currentPlayer = 0;  // 0 for player1, 1 for player2
float lastTime = 0.0f;
bool throwProcessed = false; //so multiple frames cant register click

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Dartboard", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW!" << std::endl;
        return -1;
    }

    Background background("barBackground.jpg"); // Load the background texture
    Dartboard dartboard("Dartboard.png", "basic.vert", "basic.frag");
    crosshair.initialize();  // Initialize the crosshair

    // Set up the orthographic projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);  // Orthographic projection for 2D rendering
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Disable depth testing for 2D rendering
    glDisable(GL_DEPTH_TEST);

    // Enable blending (optional, for transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Process input
        processInput(window);
        updateGame(deltaTime, window, dartboard);

        glClear(GL_COLOR_BUFFER_BIT);

        background.render();
        checkOpenGLError("Background rendering");

        dartboard.render();
        checkOpenGLError("Dartboard rendering");

        crosshair.render();
        checkOpenGLError("Crosshair rendering");

        // Check for OpenGL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "OpenGL Error: " << err << std::endl;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    // Example input processing for changing crosshair position (based on mouse)
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Get the actual window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert mouse position to OpenGL coordinates (y is flipped)
    crosshair.setPosition(mouseX / width * 2.0f - 1.0f, 1.0f - mouseY / height * 2.0f);

    // Switch players when 'Enter' is pressed (just an example)
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        // Switch players (just an example, modify logic as needed)
        currentPlayer = (currentPlayer == 0) ? 1 : 0;
    }
}

void updateGame(float deltaTime, GLFWwindow* window, Dartboard& dartboard) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !throwProcessed) {
        Player& current = (currentPlayer == 0) ? player1 : player2;
        processThrow(current, dartboard);
        throwProcessed = true; // Mark throw as processed
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        throwProcessed = false; // Reset when the button is released
    }

    // Update crosshair shaking
    crosshair.update(deltaTime);
}

// Toggle function
void toggleCursor(GLFWwindow* window, bool visible) {
    if (visible) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
}

void processThrow(Player& currentPlayer, Dartboard& dartboard) {
    float hitX = crosshair.getX();
    float hitY = crosshair.getY();

    int points = dartboard.calculateScore(hitX, hitY);

    // Record the hit position
    dartboard.recordHit(hitX, hitY);

    std::cout << currentPlayer.getName() << " hit (" << hitX << ", " << hitY
        << ") and scored " << points << " points!\n";
    std::cout << "Total Score: " << currentPlayer.getScore() + points << std::endl;

    currentPlayer.addScore(points);
    currentPlayer.throwDart();
}

void checkOpenGLError(const char* description) {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (err) {
        case GL_INVALID_ENUM:
            error = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "GL_OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            error = "Unknown error";
            break;
        }
        std::cout << "OpenGL Error (" << error << "): " << description << std::endl;
    }
}



