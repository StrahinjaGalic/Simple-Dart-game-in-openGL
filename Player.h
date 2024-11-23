#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Player {
private:
    std::string name;
    int score;
    int dartsLeft;
public:
    Player(std::string name = "Player");
    void resetDarts();          // Reset darts at the start of a round
    void throwDart();           // Reduce remaining darts
    void addScore(int points);  // Add points to the player's score
    int getScore() const;       // Get player's current score
    int getDartsLeft() const;   // Get darts remaining
    std::string getName() const; // Get player name

};

#endif

