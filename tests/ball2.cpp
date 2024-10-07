// ball demo written by chatGPT
#include <chrono>
#include <iostream>
#include <thread>

#include "../dpp.hpp" // this library provides floating-point types and utilities

using namespace dpp::literals;

using D = dpp::d32; // Alias for a 32-bit floating-point type from dpp

// ANSI escape codes for clearing the screen and managing cursor visibility
auto& CLEAR_SCREEN = "\033[2J";   // Clear the entire terminal screen
auto& HIDE_CURSOR = "\033[?25l";  // Hide the terminal cursor
auto& SHOW_CURSOR = "\033[?25h";  // Show the terminal cursor

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

constexpr D gravity = 1.81;  // Gravity constant (m/s^2)
constexpr D bounce_efficiency = .8; // Percentage of energy retained after each bounce
constexpr D time_step = .02; // Time step in seconds (for simulation accuracy)
constexpr D threshold_velocity = 2; // Velocity threshold to stop the simulation

int main() {
  const auto width = 40;   // Width of the terminal "screen" (columns)
  const auto height = 20;  // Height of the terminal "screen" (rows)
  const auto delayMs = 5; // Time between frames in milliseconds (controls ball speed)

  D ballY = 1;
  D velocity = 0;

  // Hide cursor and clear the screen before starting the animation
  std::cout << HIDE_CURSOR << CLEAR_SCREEN;

  // Main loop simulating the ball motion
  for (;;)
  {
    clearBall(1, ballY);

    // Update velocity and height using basic physics equations
    velocity += gravity * time_step; // Gravity pulls the ball downwards (increasing velocity)
    ballY += velocity * time_step;  // Update the height based on the current velocity

    // Check if the ball hits the ground
    if (ballY >= height) {
      ballY = height; // Clamp height to zero (ground level)
      velocity = -velocity * bounce_efficiency; // Reverse the velocity with energy loss
      if (std::abs(velocity) < threshold_velocity) break;
    }

    drawBall(1, ballY);

    // Ensure the ball is drawn immediately by flushing the output buffer
    std::cout.flush();

    // Pause for a short period to simulate real-time animation (optional)
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
  }

  // Restore the cursor when exiting (this will not be reached in this infinite loop)
  std::cout << SHOW_CURSOR;

  return 0;
}
