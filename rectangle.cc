// -----------------------------------------------------------
//  File: rectangle.cc
//  Author: Gregory Rehbein (Modernized to C++23)
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#include "rectangle.h"

#include <cassert>
#include <ranges>

#include "global.h"

Rectangle::Rectangle(int x0, int y0, int x1, int y1)
  : Rectangle(x0, y0, x1, y1, Global::weight_of_rectangle(x0, y0, x1, y1))
{
}

Rectangle::Rectangle(int x0, int y0, int x1, int y1, std::size_t weight)
  : bounds_(std::make_tuple(x0, y0, x1, y1)),
    area_(static_cast<std::size_t>((y1 - y0 + 1) * (x1 - x0 + 1))),
    weight_(weight),
    weight_to_cost_ratio_(static_cast<double>(weight_) / (10.0 + static_cast<double>(area_))),
    label_('\0'),
    span_(),
    span_initialized_(false)
{
  assert(area_ > 0);
}

void Rectangle::make_span()
{
  if (span_initialized_) {
    return;
  }

  span_.resize(Global::num_rows * Global::num_columns);
  const auto [x0, y0, x1, y1] = bounds_;

  // Modern C++23 approach using ranges
  for (auto x : std::views::iota(x0, x1 + 1)) {
    for (auto y : std::views::iota(y0, y1 + 1)) {
      span_.set(Global::num_columns * static_cast<std::size_t>(x) +
                static_cast<std::size_t>(y));
    }
  }

  span_initialized_ = true;
}

bool Rectangle::better(const Rectangle* rect1, const Rectangle* rect2)
{
  assert(rect1 != nullptr);
  assert(rect2 != nullptr);
  return *rect1 < *rect2;
}