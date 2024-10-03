// ball demo written by chatGPT
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip> // For std::setprecision

#include "../dpp.hpp"

using namespace dpp::literals;

// ANSI escape codes for clearing screen and positioning cursor
const std::string CLEAR_SCREEN = "\033[2J";
const std::string HIDE_CURSOR = "\033[?25l";
const std::string SHOW_CURSOR = "\033[?25h";

// ANSI escape code for setting cursor position
std::string setCursorPosition(float x, float y) {
  return "\033[" + std::to_string(static_cast<int>(y)) + ";" + std::to_string(static_cast<int>(x)) + "H";
}

void drawBall(float x, float y) {
  std::cout << setCursorPosition(x, y) << "O"; // Draw the ball at position (x, y)
}

void clearBall(float x, float y) {
  std::cout << setCursorPosition(x, y) << " "; // Clear the ball's previous position
}

void onEnter()
{
  std::getchar();

  // Show cursor when the program exits
  std::cout << SHOW_CURSOR;

  std::exit(0);
}

int main() {
  const int width = 40;   // Width of the "screen"
  const int height = 20;  // Height of the "screen"
  const int delayMs = 50; // Delay between frames (in milliseconds)

  auto ballX = 20.0_d32;    // Initial x position of the ball (in float)
  auto ballY = 10.0_d32;    // Initial y position of the ball (in float)
  auto velocityX = 0.5_d32;  // Horizontal velocity of the ball (in float)
  auto velocityY = 0.3_d32;  // Vertical velocity of the ball (in float)

  std::thread keyThread(onEnter);

  // Hide cursor
  std::cout << HIDE_CURSOR << CLEAR_SCREEN;

  for (;;)
  {
    // Clear the current position of the ball
    clearBall(ballX, ballY);

    // Update ball's position based on its velocity
    ballX += velocityX;
    ballY += velocityY;

    // Check for collisions with the walls and bounce
    if (ballX <= 1.0_d32 || ballX >= width) {
        velocityX = -velocityX; // Reverse horizontal direction
    }
    if (ballY <= 1.0_d32 || ballY >= height) {
        velocityY = -velocityY; // Reverse vertical direction
    }

    // Draw the ball at the new position
    drawBall(ballX, ballY);

    // Flush output to ensure the ball is drawn immediately
    std::cout.flush();

    // Wait for a short duration before the next frame
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
  }

  return 0;
}
