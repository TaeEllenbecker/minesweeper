# Minesweeper

A basic Minesweeper application made in FLTK C++
- Includes algorithms to find mines, free spaces, number of bombs touching

# Requirements
- C++11 or higher
- FLTK (Fast Light Toolkit)

# Installation

For FLTK installation instructions visit: https://github.com/fltk/fltk

For C++
- Make sure compiler supports C++11 or higher
  - GCC
  - Clang
  - MSVC (VS Code)

After downloading the source code:
- Build the program using: g++ main.cpp -o minesweeper $(fltk-config --cxxflags --ldflags) -isysroot $(xcrun --show-sdk-path)
- Run the project: ./minesweeper
