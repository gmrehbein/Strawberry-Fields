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

using std::make_pair;

Rectangle::Rectangle(int x0, int y0, int x1, int y1)
  : Rectangle(x0, y0, x1, y1, Global::weightOfRectangle(x0, y0, x1, y1))
{
}

Rectangle::Rectangle(int x0, int y0,
                     int x1, int y1,int weight)
: bounds_(std::make_tuple(x0,y0,x1,y1)),
   area_((y1 - y0 + 1)*(x1 - x0 + 1)),
   weight_(weight)
{
  assert(area_ > 0);
  weightToCostRatio_ = double(weight_)/(10 + area_);
}

void Rectangle::makeSpan()
{
  if (spun_) return;

  span_.resize(Global::numRows * Global::numColumns);
  auto [x0,y0,x1,y1] = bounds_;
  for (int x = x0; x <= x1; ++x) {
      for (int y = y0; y <= y1; ++y) {
          span_.set(Global::numColumns * x + y);
      }
   }

  spun_ = true;
}

bool Rectangle::better(const Rectangle* rect1, const Rectangle* rect2)
{
  return *rect1 < *rect2;
}
