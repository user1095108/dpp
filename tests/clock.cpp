// clock demo written by chatGPT
#include <cmath>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>
#include <csignal>
#include <array>
#include "../dpp.hpp"

using namespace dpp::literals;
using D = dpp::d32;

// Configuration namespace to group related constants for better readability
namespace config {
  static inline D PI = D(3.14159265358979323846264338327950288);
  static constexpr int WIDTH = 40;   // Width of the terminal clock display
  static constexpr int HEIGHT = 20;  // Height of the terminal clock display
  static constexpr int RADIUS = std::min(WIDTH, HEIGHT) / 2 - 1;  // Radius for clock face

  // ANSI escape sequences for terminal control
  auto& CLEAR_SCREEN = "\033[2J";
  auto& HIDE_CURSOR = "\033[?25l";
  auto& SHOW_CURSOR = "\033[?25h";
  auto& HOME_CURSOR = "\033[H";

  // Characters used for different clock hands and border
  static constexpr char HOUR_HAND = 'H';
  static constexpr char MINUTE_HAND = 'M';
  static constexpr char SECOND_HAND = 'S';
  static constexpr char CLOCK_BORDER = '+';
}

// Global flag for graceful shutdown upon receiving a signal (e.g., CTRL+C)
volatile sig_atomic_t g_running = 1;

// Signal handler function to set the flag for a clean shutdown
extern "C" void signalHandler(int) noexcept { g_running = 0; }

class TerminalClock {
  // 2D buffer to represent the clock face in the terminal
  std::array<std::array<char, config::WIDTH>, config::HEIGHT> buffer;

  // Function to set the cursor position in the terminal
  static std::string setCursorPosition(int x, int y) {
    return "\033[" + std::to_string(y) + ';' + std::to_string(x) + 'H';
  }

  // Clears the buffer by filling it with empty spaces
  void clearBuffer() {
    for (auto& row : buffer) row.fill(' ');
  }

  // Draws the static clock face, including border and hour markers
  void drawClockFace() {
    // Draw circle border using trigonometric calculations
    for (D angle = 0; angle < 2 * config::PI; angle += 0.1) {
      int x = static_cast<int>(round(config::WIDTH / 2 + config::RADIUS * std::cos(angle)));
      int y = static_cast<int>(round(config::HEIGHT / 2 + config::RADIUS * std::sin(angle)));

      if (x >= 0 && x < config::WIDTH && y >= 0 && y < config::HEIGHT) {
        buffer[y][x] = config::CLOCK_BORDER;
      }
    }

    // Draw hour markers at each multiple of 30 degrees (for 12-hour markers)
    for (int i = 0; i < 12; ++i) {
      D angle = i * config::PI / 6;
      int x = static_cast<int>(round(config::WIDTH / 2 + (config::RADIUS - 1) * std::cos(angle)));
      int y = static_cast<int>(round(config::HEIGHT / 2 + (config::RADIUS - 1) * std::sin(angle)));

      if (x >= 0 && x < config::WIDTH && y >= 0 && y < config::HEIGHT) {
        auto const str(std::to_string(i + 3 > 12 ? i - 9 : i + 3));
        for (auto const c: str) buffer[y][x++] = c;
      }
    }
  }

  // Draws a hand (hour, minute, or second) based on its angle and length
  void drawHand(D angle, char symbol, D length_factor = 1.0) {
    D x_start = config::WIDTH / 2;
    D y_start = config::HEIGHT / 2;
    D length = config::RADIUS * length_factor;

    for (D t = 0; t < length; t += 0.5) {
      int x = static_cast<int>(round(x_start + t * std::cos(angle)));
      int y = static_cast<int>(round(y_start + t * std::sin(angle)));

      if (x >= 0 && x < config::WIDTH && y >= 0 && y < config::HEIGHT) {
        buffer[y][x] = symbol;
      }
    }
  }

public:
  // Constructor initializes the clock face
  TerminalClock() {
    clearBuffer();
    drawClockFace();
  }

  // Updates the clock hands based on the current time
  void update(const std::chrono::system_clock::time_point& now) {
    auto time = std::chrono::system_clock::to_time_t(now);
    auto* timeinfo = std::localtime(&time);

    // Calculate angles for each hand based on the current time
    D hour_angle = (timeinfo->tm_hour % 12 + timeinfo->tm_min / 60.0) * config::PI / 6 - config::PI / 2;
    D minute_angle = timeinfo->tm_min * config::PI / 30 - config::PI / 2;
    D second_angle = timeinfo->tm_sec * config::PI / 30 - config::PI / 2;

    // Temporary buffer to restore the original clock face after drawing hands
    auto const temp_buffer = buffer;

    // Draw each hand with different lengths
    drawHand(hour_angle, config::HOUR_HAND, 0.5);
    drawHand(minute_angle, config::MINUTE_HAND, 0.7);
    drawHand(second_angle, config::SECOND_HAND, 0.9);

    // Move cursor to home position and render buffer
    std::cout << config::HOME_CURSOR;
    for (const auto& row : buffer) {
      for (char c : row) std::cout << c;
      std::cout << '\n';
    }
    std::cout.flush();

    // Restore the original buffer without hands for the next update
    buffer = temp_buffer;
  }
};

int main() {
  try {
    // Set up signal handling for clean exit
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Initialize the terminal display
    std::cout << config::HIDE_CURSOR << config::CLEAR_SCREEN;

    TerminalClock clock;

    // Main loop to keep the clock updating every second
    while (g_running) {
      auto const now = std::chrono::system_clock::now();
      clock.update(now);

      // Wait until the next second
      std::this_thread::sleep_until(
        std::chrono::ceil<std::chrono::seconds>(now));
    }

    // Cleanup: show the cursor when exiting
    std::cout << config::SHOW_CURSOR;
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cout << config::SHOW_CURSOR;
    return 1;
  }
}
