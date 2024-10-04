// ball demo written by chatGPT
#include <chrono>
#include <iostream>
#include <thread>

#include "../dpp.hpp" // this library provides floating-point types and utilities

using namespace dpp::literals;

using D = dpp::d32; // Alias for a 32-bit floating-point type from dpp

// ANSI escape codes for clearing the screen and managing cursor visibility
auto CLEAR_SCREEN = "\033[2J";   // Clear the entire terminal screen
auto HIDE_CURSOR = "\033[?25l";  // Hide the terminal cursor
auto SHOW_CURSOR = "\033[?25h";  // Show the terminal cursor

// Function to set the cursor position using ANSI escape codes
// x: Horizontal position (column), y: Vertical position (row)
std::string setCursorPosition(D const x, D const y) {
  return "\033[" + std::to_string(int(y)) + ";" + std::to_string(int(x)) + "H";
}

// Function to draw the ball at position (x, y) in the terminal
void drawBall(D const x, D const y) {
  std::cout << setCursorPosition(x, y) << "O"; // Display 'O' representing the ball
}

// Function to clear the ball from its previous position (x, y)
void clearBall(D const x, D const y) {
  std::cout << setCursorPosition(x, y) << " "; // Replace 'O' with a space to "clear" the ball
}

int main() {
  const int width = 40;   // Width of the terminal "screen" (columns)
  const int height = 20;  // Height of the terminal "screen" (rows)
  const int delayMs = 50; // Time between frames in milliseconds (controls ball speed)

  D ballX = 20;          // Initial horizontal position of the ball
  D ballY = 10;          // Initial vertical position of the ball
  D velocityX = .5_d32;  // Horizontal velocity of the ball (change in x per frame)
  D velocityY = .3_d32;  // Vertical velocity of the ball (change in y per frame)

  // Hide cursor and clear the screen before starting the animation
  std::cout << HIDE_CURSOR << CLEAR_SCREEN;

  // Main loop for updating ball position and redrawing it
  for (;;)
  {
    // Clear the ball from its current position before moving it
    clearBall(ballX, ballY);

    // Check for horizontal collisions (left and right walls)
    if (ballX < 2 || ballX >= width) velocityX = -velocityX; // Invert horizontal velocity on collision

    // Check for vertical collisions (top and bottom walls)
    if (ballY < 2 || ballY >= height) velocityY = -velocityY; // Invert vertical velocity on collision

    // Update ball position based on its velocity
    ballX += velocityX;
    ballY += velocityY;

    // Clamp the ball's position within the screen bounds (ensures the ball stays in the visible area)
    ballX = std::clamp(ballX, D(1), D(width));
    ballY = std::clamp(ballY, D(1), D(height));

    // Draw the ball at its new position
    drawBall(ballX, ballY);

    // Ensure the ball is drawn immediately by flushing the output buffer
    std::cout.flush();

    // Introduce a delay between frames to control the ball's speed
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
  }

  // Restore the cursor when exiting (this will not be reached in this infinite loop)
  std::cout << SHOW_CURSOR;

  return 0; // Program exit (never reached in the infinite loop)
}
