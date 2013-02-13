// -----------------------------------------------------------
//  File: rectangle.cc
//  Author: Gregory Rehbein
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

// self
#include "rectangle.h"
#include "global.h"
#include <cassert>
#include <utility>

using std::make_pair;

Rectangle::Rectangle(int topLeftRow, int topLeftColumn,
                     int bottomRightRow, int bottomRightColumn)
  :m_topLeft(make_pair(topLeftRow, topLeftColumn)),
   m_bottomRight(make_pair(bottomRightRow, bottomRightColumn)),
   m_area(((bottomRightColumn - topLeftColumn) + 1)*((bottomRightRow - topLeftRow) + 1)),
   m_weight(Global::weightOfRectangle(topLeftRow, topLeftColumn, bottomRightRow, bottomRightColumn)),
   m_spun(false)
{
  assert(m_area > 0);
  m_weightToCostRatio = double(m_weight)/(10 + m_area);
}

Rectangle::Rectangle(int topLeftRow, int topLeftColumn,
                     int bottomRightRow, int bottomRightColumn,int weight)
  :m_topLeft(make_pair(topLeftRow, topLeftColumn)),
   m_bottomRight(make_pair(bottomRightRow, bottomRightColumn)),
   m_area(((bottomRightColumn - topLeftColumn) + 1)*((bottomRightRow - topLeftRow) + 1)),
   m_weight(weight), m_spun(false)
{
  assert(m_area > 0);
  m_weightToCostRatio = double(m_weight)/(10 + m_area);
}

Rectangle::~Rectangle()
{
}

void Rectangle::makeSpan()
{
  if (!m_spun) {
    m_span.resize(Global::numRows*Global::numColumns);
    for (int i = m_topLeft.first; i <= m_bottomRight.first; ++i)
      for (int j = m_topLeft.second; j <= m_bottomRight.second; ++j)
        m_span.set(i*Global::numColumns + j);
    m_spun = true;
  }
}

bool Rectangle::better(const Rectangle* r1, const Rectangle* r2)
{
  return *r1 < *r2;
}
