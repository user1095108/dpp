// ball demo written by chatGPT
#include <chrono>
#include <iostream>
#include <thread>

#include "../dpp.hpp"

using namespace dpp::literals;

using D = dpp::d32;

// ANSI escape codes for clearing screen and positioning cursor
const std::string CLEAR_SCREEN = "\033[2J";
const std::string HIDE_CURSOR = "\033[?25l";
const std::string SHOW_CURSOR = "\033[?25h";

// ANSI escape code for setting cursor position
std::string setCursorPosition(D const x, D const y) {
  return "\033[" + std::to_string(int(y)) + ";" + std::to_string(int(x)) + "H";
}

void drawBall(D const x, D const y) {
  std::cout << setCursorPosition(x, y) << "O"; // Draw the ball at position (x, y)
}

void clearBall(D const x, D const y) {
  std::cout << setCursorPosition(x, y) << " "; // Clear the ball's previous position
}

int main() {
  const int width = 40;   // Width of the "screen"
  const int height = 20;  // Height of the "screen"
  const int delayMs = 50; // Delay between frames (in milliseconds)

  D ballX = 20;          // Initial x position of the ball (in float)
  D ballY = 10;          // Initial y position of the ball (in float)
  D velocityX = .5_d32;  // Horizontal velocity of the ball (in float)
  D velocityY = .3_d32;  // Vertical velocity of the ball (in float)

  // Hide cursor
  std::cout << HIDE_CURSOR << CLEAR_SCREEN;

  for (;;)
  {
    // Clear the current position of the ball
    clearBall(ballX, ballY);

    // Check for collisions with the walls and bounce
    if (ballX <= 1 || ballX >= width) {
      velocityX = -velocityX; // Reverse horizontal direction
      ballX = (ballX <= 1 ? 2 : width - 1) - velocityX;
    }

    if (ballY <= 1 || ballY >= height) {
      velocityY = -velocityY; // Reverse vertical direction
      ballY = (ballY <= 1 ? 2 : height - 1) - velocityY;
    }

    // Update ball's position based on its velocity
    ballX += velocityX;
    ballY += velocityY;

    // Draw the ball at the new position
    drawBall(ballX, ballY);

    // Flush output to ensure the ball is drawn immediately
    std::cout.flush();

    // Wait for a short duration before the next frame
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
  }

  return 0;
}
