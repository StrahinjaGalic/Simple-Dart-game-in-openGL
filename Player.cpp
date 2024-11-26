#include "Player.h"

Player::Player(std::string name) : name(name), score(0), dartsLeft(3) {}

void Player::resetDarts() {
    dartsLeft = 3;
}

void Player::throwDart() {
    if (dartsLeft > 0) dartsLeft--;
}

void Player::addScore(int points) {
    if (score + points <= 501) {
        score += points;
    }    
}

int Player::getScore() const {
    return score;
}

int Player::getDartsLeft() const {
    return dartsLeft;
}

std::string Player::getName() const {
    return name;
}
