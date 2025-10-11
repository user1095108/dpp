#include <execution>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

#include "../dpp.hpp"

using namespace dpp::literals;

using D = dpp::d32;

// ---------- compile-time constants ----------
constexpr int    WIDTH  = 80;          // console columns
constexpr int    HEIGHT = 24;          // console rows
constexpr D SCALE  = 40_d32;           // zoom factor
constexpr D Z_DIST = 5.7_d32;          // camera distance
constexpr D THETA  = 0.05_d32;         // rotation speed (radians per frame)

// ---------- basic 3-D math ----------
struct Vec3 { D x{}, y{}, z{}; };
struct Vec2 { int x{}, y{}; };

using Mat3 = std::array<std::array<D, 3>, 3>;

inline Vec3 operator*(const Mat3& m, const Vec3& v) noexcept {
    return { m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
             m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
             m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z };
}

Mat3 rotationMatrix(D const ax, D const ay) noexcept {
    const D cx = std::cos(ax), sx = std::sin(ax);
    const D cy = std::cos(ay), sy = std::sin(ay);
    return {  cy,  sx*sy,  cx*sy,
              0,    cx,   -sx,
             -sy,  sx*cy,  cx*cy };
}

// ---------- cube geometry ----------
const std::array<Vec3,8> cubeVertices = {{
    {-1,-1,-1}, { 1,-1,-1}, { 1, 1,-1}, {-1, 1,-1},
    {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1}
}};

const std::array<std::pair<int,int>,12> cubeEdges = {{
    {0,1},{1,2},{2,3},{3,0},   // back face
    {4,5},{5,6},{6,7},{7,4},   // front face
    {0,4},{1,5},{2,6},{3,7}    // connecting edges
}};

// ---------- rasterisation helpers ----------
void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}

Vec2 project(const Vec3& v) noexcept {
    const D z = v.z + Z_DIST;
    return { int(SCALE * v.x / z + WIDTH  / 2),
             int(SCALE * v.y / z + HEIGHT / 2) };
}

// ---------- main loop ----------
int main() {
    for (D angleX{}, angleY{};;) {
        // --- transform vertices, project to 2D ---
        std::array<Vec2, cubeVertices.size()> screen;

        std::ranges::transform(
            cubeVertices,
            screen.begin(),
            [R = rotationMatrix(angleX, angleY)](auto const& p) noexcept
            {
              return project(R * p);
            }
        );

        // --- raster buffer ---
        std::vector<std::string> buf(HEIGHT, std::string(WIDTH, ' '));

        // --- draw edges ---
        for (auto const [i, j] : cubeEdges) {
          auto const& [a, b](std::tie(screen[i], screen[j]));

          int x0 = a.x, y0 = a.y;
          int const x1 = b.x, y1 = b.y;
          int const dx = std::abs(x1 - x0);
          int const dy = std::abs(y1 - y0);
          int const sx = (x0 < x1) - (x0 > x1);
          int const sy = (y0 < y1) - (y0 > y1);
          int err = dx - dy;

          for (;;) {
              if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT)
                  buf[y0][x0] = '#';
              if (x0 == x1 && y0 == y1) break;
              int const e2 = 2 * err;
              if (e2 > -dy) { err -= dy; x0 += sx; }
              if (e2 <  dx) { err += dx; y0 += sy; }
          }
        }

        // --- present frame ---
        clearScreen();
        for (const auto& row : buf) std::cout << row << '\n';
        std::cout.flush();

        // --- update rotation ---
        angleX += THETA;
        angleY += THETA * 0.7_d32;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}
