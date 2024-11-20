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
void updateGame(float deltaTime);

// Game objects
Player player1("Player 1");
Player player2("Player 2");
Crosshair crosshair;

int currentPlayer = 0;  // 0 for player1, 1 for player2
float lastTime = 0.0f;

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

        // Update game state (e.g., crosshair shaking, player actions)
        updateGame(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);

        background.render();
        dartboard.render();
        crosshair.render();
        

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

void updateGame(float deltaTime) {
    // Update crosshair shaking
    crosshair.update(deltaTime);

    // Example: update game logic like player actions, scores, etc.
    // For now, just print the current player and their darts left.
    Player* current = (currentPlayer == 0) ? &player1 : &player2;
    std::cout << current->getName() << " - Darts Left: " << current->getDartsLeft() << std::endl;
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
