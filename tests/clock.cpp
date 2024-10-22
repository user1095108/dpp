// clock demo written by chatGPT
#include <cmath>          // For mathematical functions like cos(), sin(), and M_PI (constant for π)
#include <chrono>         // For time-related functions to get current time and handle durations
#include <iostream>       // For input/output operations (like std::cout)
#include <thread>         // For using sleep_for() to pause the program execution

#include "../dpp.hpp"     // External library that provides floating-point types and utilities

using namespace dpp::literals; // Using literals from the dpp library

using D = dpp::d32; // Alias for a 32-bit floating-point type from the dpp library

#ifndef D_PI
# define D_PI D(3.14159265358979323846264338327950288) // Definition of π if not already defined
#endif // D_PI

// Constants for the terminal dimensions and clock radius
const auto width = 40;   // Width of the terminal "screen" (number of columns)
const auto height = 20;  // Height of the terminal "screen" (number of rows)
const auto radius = height / 2; // Radius of the clock (half of the terminal height)

// ANSI escape codes for controlling the terminal screen and cursor visibility
auto& CLEAR_SCREEN = "\033[2J";   // Code to clear the entire terminal screen
auto& HIDE_CURSOR = "\033[?25l";  // Code to hide the terminal cursor
auto& SHOW_CURSOR = "\033[?25h";  // Code to show the terminal cursor again

// Function to set the cursor position at (x, y) in the terminal using ANSI escape codes
std::string setCursorPosition(D const x, D const y) {
  return "\033[" + std::to_string(int(y)) + ";" + std::to_string(int(x)) + "H"; // Format cursor position
}

// Function to draw the current second hand on the clock at the appropriate position
// s: seconds (0-59), c: character to draw (e.g., 'O' for marking the second hand)
void drawSeconds(int sc, char const ch) noexcept
{
  // Calculate the angle for the second hand, based on the input seconds value
  auto const phi(D_PI / 30 * sc); // Each second corresponds to 6 degrees (π/30 radians)

  // Draw the character 'c' at the calculated (x, y) position based on phi
  std::cout <<
    setCursorPosition(
      std::round(width / 2 + radius * std::sin(phi) + 1),  // Calculate x-coordinate
      std::round(height / 2 - radius * std::cos(phi) + 1)  // Calculate y-coordinate
    ) << ch; // Output the character at the specified position
}

// Function to draw the current hour hand on the clock at the appropriate position
// h: hours (0-23), c: character to draw
void drawHours(int h, char const ch) noexcept
{
  // Calculate the angle for the hour hand
  auto const phi(D_PI / 6 * h); // Each hour corresponds to 30 degrees (π/6 radians)

  // Draw the character 'c' at the calculated (x, y) position based on phi
  std::cout <<
    setCursorPosition(
      std::round(width / 2 + radius * std::sin(phi) + 1),  // Calculate x-coordinate
      std::round(height / 2 - radius * std::cos(phi) + 1)  // Calculate y-coordinate
    ) << ch; // Output the character at the specified position
}

int main()
{
  // Clear the screen and hide the cursor before starting the clock
  std::cout << HIDE_CURSOR << CLEAR_SCREEN;

  // Variables to hold the current second, minute, and hour
  int s{}, m{}, h{};

  // Infinite loop to continuously update the clock every second
  for (;;)
  {
    // Convert the current time to a local time zone
    auto const lnow(std::chrono::zoned_time{
      std::chrono::current_zone(), std::chrono::system_clock::now()}); // Get the local time
    auto const ltp(lnow.get_local_time()); // Get the local time point

    // Extract hours, minutes, and seconds from the current local time
    std::chrono::hh_mm_ss const tod{ltp -
      std::chrono::floor<std::chrono::days>(ltp)}; // Get the time of day (hh:mm:ss)

    // Clear the previous positions of the hands
    drawSeconds(s, ' ');                      // Clear previous second hand
    drawSeconds(m, ' ');                      // Clear previous minute hand
    drawHours(h, ' ');                        // Clear previous hour hand

    // Draw current second hand
    drawSeconds(s = tod.seconds().count(), 'O');  // Update and draw current second hand
    drawSeconds(m = tod.minutes().count(), 'O');  // Update and draw current minute hand
    drawHours(h = tod.hours().count(), 'O');  // Update and draw current hour hand

    // Ensure the drawn output is immediately flushed to the terminal
    std::cout.flush();

    // Sleep for 1 second before updating the clock again
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // After exiting the loop (not reachable in this case), show the cursor again
  std::cout << SHOW_CURSOR;

  return 0; // Return from main function
}
