// clock demo written by chatGPT
#include <cmath>          // For mathematical functions like cos() and sin()
#include <algorithm>      // For utilities like std::min()
#include <chrono>         // For time functions (e.g., current time and durations)
#include <iostream>       // For input/output operations (e.g., std::cout)
#include <thread>         // For sleep_for() to pause program execution

#include "../dpp.hpp"     // External library for floating-point types and utilities

using namespace dpp::literals; // Import custom literals from the dpp library

using D = dpp::d32; // Alias for a 32-bit floating-point type from the dpp library

#ifndef D_PI
# define D_PI D(3.14159265358979323846264338327950288) // Define the constant for π if not already defined
#endif // D_PI

// Constants for terminal dimensions and clock radius
const auto width = 40;   // Terminal width in characters (number of columns)
const auto height = 20;  // Terminal height in characters (number of rows)
const auto radius = std::min(width, height) / 2; // Clock radius based on terminal size

// ANSI escape codes for controlling terminal screen and cursor visibility
auto& CLEAR_SCREEN = "\033[2J";   // Escape code to clear the terminal screen
auto& HIDE_CURSOR = "\033[?25l";  // Escape code to hide the terminal cursor
auto& SHOW_CURSOR = "\033[?25h";  // Escape code to show the terminal cursor

// Function to set the cursor position at (x, y) in the terminal using ANSI escape codes
std::string setCursorPosition(D const& x, D const& y) {
  return "\033[" + std::to_string(int(y)) + ';' + std::to_string(int(x)) + 'H'; // ANSI code for cursor positioning
}

// Function to draw the clock hand (second, minute, or hour) at the appropriate position
// phi: angular velocity, h: current time unit (seconds, minutes, or hours), ch: character representing the hand
void drawHand(D const phi, int const h, char const ch) noexcept
{
  auto const angle(phi * h); // Calculate the angle for the current time unit

  // Calculate the (x, y) position of the clock hand and move the cursor there to draw the character
  std::cout <<
    setCursorPosition(
      round(D(width + 1) / 2 + radius * std::sin(angle)),  // Calculate x-coordinate (horizontal position)
      round(D(height + 1) / 2 - radius * std::cos(angle))  // Calculate y-coordinate (vertical position)
    ) << ch; // Output the character representing the clock hand at the calculated position
}

int main()
{
  // Clear the terminal and hide the cursor to prepare for clock display
  std::cout << HIDE_CURSOR << CLEAR_SCREEN;

  // Infinite loop to continuously update the clock every second
  for (int s{}, m{}, h{};;) // Variables to store current seconds, minutes, and hours
  {
    // Get the current local time in the system's time zone
    auto const lnow(std::chrono::zoned_time{
      std::chrono::current_zone(), std::chrono::system_clock::now()}); // Obtain current local time
    auto const ltp(lnow.get_local_time()); // Get the local time point

    // Extract hours, minutes, and seconds from the current local time
    std::chrono::hh_mm_ss const tod{ltp -
      std::chrono::floor<std::chrono::days>(ltp)}; // Calculate time of day (ignoring date)

    // Erase the previous positions of the clock hands by drawing spaces
    drawHand(D_PI / 30, s, ' ');                      // Clear previous second hand position
    drawHand(D_PI / 30, m, ' ');                      // Clear previous minute hand position
    drawHand(D_PI / 6, h, ' ');                       // Clear previous hour hand position

    // Draw the clock hands at the updated current time
    drawHand(D_PI / 30, s = tod.seconds().count(), 'O');  // Draw second hand ('O') at the current second
    drawHand(D_PI / 30, m = tod.minutes().count(), 'O');  // Draw minute hand ('O') at the current minute
    drawHand(D_PI / 6, h = tod.hours().count(), 'O');     // Draw hour hand ('O') at the current hour

    // Flush output to update the terminal immediately
    std::cout.flush();

    // Pause for 1 second before updating the clock again
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Restore terminal cursor visibility when the program ends (though it never ends in this case)
  std::cout << SHOW_CURSOR;

  return 0; // Exit the main function (never reached due to infinite loop)
}
