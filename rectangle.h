// -----------------------------------------------------------
//  File: rectangle.h
//  Author: Gregory Rehbein
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#pragma once

#include <tuple>
#include <boost/dynamic_bitset.hpp>

class Rectangle
{
  std::tuple<int,int,int,int> bounds_;
  size_t area_;
  size_t weight_;
  double weightToCostRatio_;

  char label_;
  boost::dynamic_bitset<> span_;
  bool spun_ = false;

public:

  Rectangle(int x0, int y0, int x1, int y1);
  Rectangle(int x0, int y0, int x1, int y1, int weight);

  [[nodiscard]] size_t area() const {
    return area_;
  }

  [[nodiscard]] size_t cost() const {
    return 10 + area_;
  }

  [[nodiscard]] char label() const {
    return label_;
  }

  [[nodiscard]] size_t weight() const {
    return weight_;
  }

  [[nodiscard]] double weightToCostRatio() const {
    return weightToCostRatio_;
  }

  [[nodiscard]] int topLeftRow() const {
    return std::get<0>(bounds_);
  }

  [[nodiscard]] int topLeftColumn() const {
    return std::get<1>(bounds_);
  }

  [[nodiscard]] int bottomRightRow() const {
    return std::get<2>(bounds_);
  }

  [[nodiscard]] int bottomRightColumn() const {
    return std::get<3>(bounds_);
  }

  [[nodiscard]] bool intersects(const Rectangle* other) const {
    return span_.intersects(other->span_);
  }

  [[nodiscard]] bool isSubsetOf(const Rectangle* other) const {
    return span_.is_subset_of(other->span_);
  }

  [[nodiscard]] const boost::dynamic_bitset<>& span() const {
    return span_;
  }

  bool operator< (const Rectangle& other) const {
    return weightToCostRatio_ < other.weightToCostRatio_;
  }

  void setLabel(char label) {
    label_ = label;
  }

  //-------------------------------------------
  // Turn on all bits contained within the
  // rectangle's span
  //-------------------------------------------
  void makeSpan();

  //--------------------------------------------------
  // Convenience function operating on type Rectangle*.
  // Returns *rect1 < *rect2
  //---------------------------------------------------
  static bool better(const Rectangle* rect1, const Rectangle* rect2);
};
