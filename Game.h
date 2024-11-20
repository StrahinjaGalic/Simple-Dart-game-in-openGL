#ifndef GAME_H
#define GAME_H

#include "Player.h"
#include "Dartboard.h"
#include "Crosshair.h"

class Game {
private:
    Player player1;
    Player player2;
    Dartboard dartboard;
    Crosshair crosshair;

    Player* activePlayer;
    bool isPaused;
    int roundNumber;

public:
    Game();                // Constructor
    void init();           // Initialize the game
    void update(float dt); // Update game logic
    void render();         // Render the game
    void handleInput(int key, int action); // Handle player input
    void switchTurn();     // Switch active player
};

#endif

