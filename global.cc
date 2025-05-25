// -----------------------------------------------------------
//  File: global.cc
//  Author: Gregory Rehbein (Modernized to C++23)
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#include "global.h"
#include <ranges>

std::size_t Global::weight_of_rectangle(
  int top_left_row,
  int top_left_column,
  int bottom_right_row,
  int bottom_right_column) noexcept
{
  std::size_t weight = 0;

  // Modern C++23 approach using ranges
  for (auto i : std::views::iota(top_left_row, bottom_right_row + 1)) {
    for (auto j : std::views::iota(top_left_column, bottom_right_column + 1)) {
      // optimization: use unchecked [] access
      weight += Global::field[i][j];
    }
  }

  return weight;
}