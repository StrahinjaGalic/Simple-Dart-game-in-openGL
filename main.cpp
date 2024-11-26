#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Dartboard.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include "Crosshair.h"
#include "Player.h"
#include "Background.h"
#include "TextRenderer.h"

void processInput(GLFWwindow* window);
void updateGame(float deltaTime, GLFWwindow* window, Dartboard& dartboard, TextRenderer& textRenderer);
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    TextRenderer textRenderer("Jaro-Regular.ttf", "text.vert", "text.frag", 48);
    TextRenderer nameRenderer("Jaro-Regular.ttf", "text.vert", "text.frag", 20);

    crosshair.initialize();  // Initialize the crosshair
    crosshair.setColor(1.0f, 0.0f, 0.0f);  // Start with Player 1's color (Red)

    // Set up the orthographic projection matrix
   /* glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);  // Orthographic projection for 2D rendering
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Disable depth testing for 2D rendering
    glDisable(GL_DEPTH_TEST);*/

    // Enable blending (optional, for transparency)
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Process input
        processInput(window);
        //updateGame(deltaTime, window, dartboard, textRenderer);

        glClear(GL_COLOR_BUFFER_BIT);

        background.render();
        checkOpenGLError("Background rendering");

        dartboard.render();
        checkOpenGLError("Dartboard rendering");

        crosshair.render();
        checkOpenGLError("Crosshair rendering");

        updateGame(deltaTime, window, dartboard, textRenderer);

        nameRenderer.RenderText("RA 156/2021", 0.0f, 780.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)); // Adjust position if needed
        nameRenderer.RenderText("Strahinja Galic", 0.0f, 760.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f)); // Adjust position if needed

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
    // Get the mouse cursor position in window coordinates
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Get the actual window size
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert mouse position to OpenGL normalized coordinates (-1 to 1 range)
    float normX = (mouseX / width) * 2.0f - 1.0f;  // X coordinate in NDC
    float normY = 1.0f - (mouseY / height) * 2.0f; // Y coordinate in NDC (y-axis is flipped in OpenGL)

    // Apply random shaking effect to the crosshair
    float shakeAmount = 0.02f;  // Small shake amount, adjust to taste
    float shakeX = (rand() % 1000 - 500) / 500.0f * shakeAmount;  // Random shake in X
    float shakeY = (rand() % 1000 - 500) / 500.0f * shakeAmount;  // Random shake in Y

    // Update the crosshair position with shake
    crosshair.setPosition(normX + shakeX, normY + shakeY);

    // Check for 'Enter' key press to switch players (for demonstration purposes)
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        // Switch players
        currentPlayer = (currentPlayer == 0) ? 1 : 0;
    }
}


void updateGame(float deltaTime, GLFWwindow* window, Dartboard& dartboard, TextRenderer& textRenderer) {
    static bool waitingToClear = false;
    static float clearStartTime = 0.0f;

    // Get the current player
    Player& current = (currentPlayer == 0) ? player1 : player2;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !throwProcessed) {
        if (current.getDartsLeft() > 0) { // Only process throw if player has darts left
            processThrow(current, dartboard);
            throwProcessed = true;

            // Check for win condition
            if (current.getScore() == 501) {
                std::cout << current.getName() << " wins with a perfect 501!" << std::endl;
                glfwSetWindowShouldClose(window, true); // Close the window to end the game
            }
            else if (current.getDartsLeft() == 0 && !waitingToClear) {
                // Begin waiting to clear hits
                waitingToClear = true;
                clearStartTime = glfwGetTime();
            }
        }
        else {
            std::cout << current.getName() << " has no darts left!" << std::endl;
        }
    }

    if (waitingToClear) {
        float elapsedTime = glfwGetTime() - clearStartTime;
        if (elapsedTime >= 1.0f) {
            dartboard.clearHits(); // Clear the hits
            currentPlayer = (currentPlayer == 0) ? 1 : 0; // Switch players
            (currentPlayer == 0 ? player1 : player2).resetDarts();
            waitingToClear = false;

            // Change crosshair color based on the current player
            if (currentPlayer == 0) {
                crosshair.setColor(1.0f, 0.0f, 0.0f);  // Red for Player 1
            }
            else {
                crosshair.setColor(0.0f, 0.0f, 1.0f);  // Blue for Player 2
            }

            std::cout << "Switching to " << (currentPlayer == 0 ? player1.getName() : player2.getName()) << std::endl;
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        throwProcessed = false; // Reset when the button is released
    }

    // Update crosshair shaking
    crosshair.update(deltaTime);

    // Render the player's name at the top-left corner of the screen
    std::string text = current.getName() + ": " + std::to_string(current.getScore());
    textRenderer.RenderText(text, 0.0f, 30.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    checkOpenGLError("Rendering player name text");
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



