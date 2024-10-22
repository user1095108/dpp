// clock demo written by chatGPT
#include <cmath>          // For mathematical functions like cos(), sin(), and the constant M_PI (π)
#include <chrono>         // For working with time functions like getting the current time and durations
#include <iostream>       // For input/output operations (e.g., std::cout)
#include <thread>         // For using sleep_for() to pause program execution

#include "../dpp.hpp"     // External library providing floating-point types and utilities

using namespace dpp::literals; // Importing custom literals from the dpp library

using D = dpp::d32; // Defining 'D' as an alias for a 32-bit floating-point type from the dpp library

#ifndef D_PI
# define D_PI D(3.14159265358979323846264338327950288) // Define π if not already defined
#endif // D_PI

// Constants for terminal dimensions and clock radius
const auto width = 40;   // Terminal width in characters (number of columns)
const auto height = 20;  // Terminal height in characters (number of rows)
const auto radius = height / 2; // Radius of the clock, proportional to half the terminal height

// ANSI escape codes for controlling terminal screen and cursor visibility
auto& CLEAR_SCREEN = "\033[2J";   // Escape code to clear the terminal screen
auto& HIDE_CURSOR = "\033[?25l";  // Escape code to hide the terminal cursor
auto& SHOW_CURSOR = "\033[?25h";  // Escape code to show the terminal cursor

// Function to set the cursor position at (x, y) in the terminal using ANSI escape codes
std::string setCursorPosition(int const x, int const y) {
  return "\033[" + std::to_string(y) + ";" + std::to_string(x) + "H"; // Format (x, y) as an ANSI code
}

// Function to draw the clock hand (second, minute, or hour) at the appropriate position
// phi: angular velocity, h: current time unit (seconds, minutes, or hours), ch: character to draw the hand
void drawHand(D const phi, int const h, char const ch) noexcept
{
  auto const phih(phi * h);

  // Calculate and move the cursor to the correct (x, y) position and draw the character 'ch'
  std::cout <<
    setCursorPosition(
      width / 2 + std::round(radius * std::sin(phih)) + 1,  // Calculate x-coordinate (horizontal position)
      height / 2 - std::round(radius * std::cos(phih)) + 1  // Calculate y-coordinate (vertical position)
    ) << ch; // Output the character 'ch' at the calculated position
}

int main()
{
  // Clear the terminal and hide the cursor before starting the clock display
  std::cout << HIDE_CURSOR << CLEAR_SCREEN;

  // Infinite loop to continuously update the clock once every second
  for (int s{}, m{}, h{};;) // Variables to store the current second, minute, and hour
  {
    // Get the current local time in the system's time zone
    auto const lnow(std::chrono::zoned_time{
      std::chrono::current_zone(), std::chrono::system_clock::now()}); // Obtain local time with time zone
    auto const ltp(lnow.get_local_time()); // Extract the local time point

    // Extract hours, minutes, and seconds from the current local time
    std::chrono::hh_mm_ss const tod{ltp -
      std::chrono::floor<std::chrono::days>(ltp)}; // Get the time of day (hours, minutes, and seconds)

    // Clear the previous positions of the clock hands
    drawHand(D_PI / 30, s, ' ');                      // Clear the previous second hand
    drawHand(D_PI / 30, m, ' ');                      // Clear the previous minute hand
    drawHand(D_PI / 6, h, ' ');                       // Clear the previous hour hand

    // Draw the updated clock hands for the current time
    drawHand(D_PI / 30, s = tod.seconds().count(), 'O');  // Draw the current second hand ('O')
    drawHand(D_PI / 30, m = tod.minutes().count(), 'O');  // Draw the current minute hand ('O')
    drawHand(D_PI / 6, h = tod.hours().count(), 'O');     // Draw the current hour hand ('O')

    // Flush the output to ensure the clock updates immediately
    std::cout.flush();

    // Pause execution for 1 second before updating the clock again
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Restore the terminal cursor visibility when the program ends (though the loop is infinite)
  std::cout << SHOW_CURSOR;

  return 0; // Exit the main function (not reached in this case)
}
