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

// Configuration namespace to group related constants
namespace config {
    constexpr D PI = D(3.14159265358979323846264338327950288);
    constexpr int WIDTH = 40;
    constexpr int HEIGHT = 20;
    constexpr int RADIUS = std::min(WIDTH, HEIGHT) / 2 - 1;
    
    // ANSI escape sequences
    const char* const CLEAR_SCREEN = "\033[2J";
    const char* const HIDE_CURSOR = "\033[?25l";
    const char* const SHOW_CURSOR = "\033[?25h";
    const char* const HOME_CURSOR = "\033[H";
    
    // Clock face characters
    constexpr char HOUR_HAND = 'H';
    constexpr char MINUTE_HAND = 'M';
    constexpr char SECOND_HAND = 'S';
    constexpr char CLOCK_BORDER = '+';
}

// Global flag for graceful shutdown
volatile sig_atomic_t g_running = 1;

// Signal handler for clean termination
void signalHandler(int) {
  g_running = 0;
}

class TerminalClock {
private:
  // Buffer to store clock face
  std::array<std::array<char, config::WIDTH>, config::HEIGHT> buffer;
  
  static std::string setCursorPosition(int x, int y) {
    return "\033[" + std::to_string(y) + ';' + std::to_string(x) + 'H';
  }

  void clearBuffer() {
    for (auto& row : buffer) row.fill(' ');
  }

  void drawClockFace() {
    // Draw circle border
    for (D angle = 0; angle < 2 * config::PI; angle += 0.1) {
      int x = static_cast<int>(round(config::WIDTH / 2 + config::RADIUS * std::cos(angle)));
      int y = static_cast<int>(round(config::HEIGHT / 2 + config::RADIUS * std::sin(angle)));
      
      if (x >= 0 && x < config::WIDTH && y >= 0 && y < config::HEIGHT) {
        buffer[y][x] = config::CLOCK_BORDER;
      }
    }
    
    // Draw hour markers
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
  TerminalClock() {
    clearBuffer();
    drawClockFace();
  }
  
  void update(const std::chrono::system_clock::time_point& now) {
    auto time = std::chrono::system_clock::to_time_t(now);
    auto* timeinfo = std::localtime(&time);
    
    // Calculate hand angles
    D hour_angle = (timeinfo->tm_hour % 12 + timeinfo->tm_min / 60.0) * config::PI / 6 - config::PI / 2;
    D minute_angle = timeinfo->tm_min * config::PI / 30 - config::PI / 2;
    D second_angle = timeinfo->tm_sec * config::PI / 30 - config::PI / 2;
    
    // Create temporary buffer
    auto temp_buffer = buffer;
    
    // Draw hands with different lengths
    drawHand(hour_angle, config::HOUR_HAND, 0.5);
    drawHand(minute_angle, config::MINUTE_HAND, 0.7);
    drawHand(second_angle, config::SECOND_HAND, 0.9);
    
    // Render the buffer
    std::cout << config::HOME_CURSOR;

    for (const auto& row : buffer) {
      for (char c : row) std::cout << c;
      std::cout << '\n';
    }

    std::cout.flush();

    // Restore the original buffer (without hands) for next update
    buffer = temp_buffer;
  }
};

int main() {
  try {
    // Set up signal handling
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Initialize terminal
    std::cout << config::HIDE_CURSOR << config::CLEAR_SCREEN;

    TerminalClock clock;
    
    // Main loop
    while (g_running) {
      auto const now = std::chrono::system_clock::now();
      clock.update(now);

      // Sleep until the next second
      auto next_second = std::chrono::ceil<std::chrono::seconds>(now);
      std::this_thread::sleep_until(next_second);
    }
    
    // Cleanup
    std::cout << config::SHOW_CURSOR << config::CLEAR_SCREEN;
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cout << config::SHOW_CURSOR;
    return 1;
  }
}
