// -----------------------------------------------------------
//  File: rectangle.h
//  Author: Gregory Rehbein (Modernized to C++23)
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#pragma once

#include <compare>
#include <cstddef>
#include <tuple>
#include <boost/dynamic_bitset.hpp>

class Rectangle
{
private:
  std::tuple<int, int, int, int> bounds_;
  std::size_t area_;
  std::size_t weight_;
  double weight_to_cost_ratio_;
  char label_;
  boost::dynamic_bitset<> span_;
  bool span_initialized_ = false;

public:
  Rectangle(int x0, int y0, int x1, int y1);
  Rectangle(int x0, int y0, int x1, int y1, std::size_t weight);

  // Rule of 5 (modern C++ best practice)
  ~Rectangle() = default;
  Rectangle(const Rectangle&) = default;
  Rectangle& operator=(const Rectangle&) = default;
  Rectangle(Rectangle&&) = default;
  Rectangle& operator=(Rectangle&&) = default;

  // Accessors
  [[nodiscard]] constexpr std::size_t area() const noexcept
  {
    return area_;
  }

  [[nodiscard]] constexpr std::size_t cost() const noexcept
  {
    return 10 + area_;
  }

  [[nodiscard]] constexpr char label() const noexcept
  {
    return label_;
  }

  [[nodiscard]] constexpr std::size_t weight() const noexcept
  {
    return weight_;
  }

  [[nodiscard]] constexpr double weight_to_cost_ratio() const noexcept
  {
    return weight_to_cost_ratio_;
  }

  [[nodiscard]] constexpr int top_left_row() const noexcept
  {
    return std::get<0>(bounds_);
  }

  [[nodiscard]] constexpr int top_left_column() const noexcept
  {
    return std::get<1>(bounds_);
  }

  [[nodiscard]] constexpr int bottom_right_row() const noexcept
  {
    return std::get<2>(bounds_);
  }

  [[nodiscard]] constexpr int bottom_right_column() const noexcept
  {
    return std::get<3>(bounds_);
  }

  // Relationship queries
  [[nodiscard]] bool intersects(const Rectangle* other) const
  {
    return span_.intersects(other->span_);
  }

  [[nodiscard]] bool is_subset_of(const Rectangle* other) const
  {
    return span_.is_subset_of(other->span_);
  }

  [[nodiscard]] const boost::dynamic_bitset<>& span() const noexcept
  {
    return span_;
  }

  // Comparison operators
  [[nodiscard]] constexpr auto operator<=>(const Rectangle& other) const noexcept
  {
    return weight_to_cost_ratio_ <=> other.weight_to_cost_ratio_;
  }

  [[nodiscard]] constexpr bool operator==(const Rectangle& other) const noexcept = default;

  // Mutators
  void set_label(char label) noexcept
  {
    label_ = label;
  }

  //-------------------------------------------
  // Turn on all bits contained within the
  // rectangle's span
  //-------------------------------------------
  void make_span();

  //--------------------------------------------------
  // Convenience function operating on type Rectangle*.
  // Returns *rect1 < *rect2
  //--------------------------------------------------
  static bool better(const Rectangle* rect1, const Rectangle* rect2);
};