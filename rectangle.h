// -----------------------------------------------------------
//  File: rectangle.h
//  Author: Gregory Rehbein
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <utility>
#include <boost/dynamic_bitset.hpp>

class Rectangle
{
  std::pair<int,int> m_topLeft;
  std::pair<int,int> m_bottomRight;
  size_t m_area;
  size_t m_weight;
  double m_weightToCostRatio;

  char m_label;
  boost::dynamic_bitset<> m_span;
  bool m_spun;

public:

  Rectangle(int topLeftRow, int topLeftColumn,
            int bottomRightRow, int bottomRightColumn);
  Rectangle(int topLeftRow, int topLeftColumn,
            int bottomRightRow, int bottomRightColumn, int weight);
  ~Rectangle();

  inline size_t area() const {
    return m_area;
  }
  inline size_t cost() const {
    return 10 + m_area;
  }
  inline char label() const {
    return m_label;
  }
  inline size_t weight() const {
    return m_weight;
  }
  inline double weightToCostRatio() const {
    return m_weightToCostRatio;
  }
  inline int topLeftRow() const {
    return m_topLeft.first;
  }
  inline int topLeftColumn() const {
    return m_topLeft.second;
  }
  inline int bottomRightRow() const {
    return m_bottomRight.first;
  }
  inline int bottomRightColumn() const {
    return m_bottomRight.second;
  }
  inline bool intersects(const Rectangle* other) const {
    return m_span.intersects(other->m_span);
  }
  inline bool isSubsetOf(const Rectangle* other) const {
    return m_span.is_subset_of(other->m_span);
  }
  inline const boost::dynamic_bitset<>& span() const {
    return m_span;
  }
  inline bool operator< (const Rectangle& other) const {
    return m_weightToCostRatio < other.m_weightToCostRatio;
  }
  inline void setLabel(char c) {
    m_label = c;
  }

  //-------------------------------------------
  // Turn on all bits contained within the
  // rectangle's span
  //-------------------------------------------
  void makeSpan();

  //--------------------------------------------------
  // Convenience function operating on type Rectangle*.
  // Returns *r1 < *r2
  //---------------------------------------------------
  static bool better(const Rectangle* r1, const Rectangle* r2);
};

#endif // RECTANGLE_H
