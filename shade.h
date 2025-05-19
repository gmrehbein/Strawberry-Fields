// -----------------------------------------------------------
//  File: shade.h
//  Author: Gregory Rehbein
//
//  Shade struct declaration. A Shade consists of a
//  pair of rectangles, their join, and two sets of rectangles
//  from the optimizer's rectangle result set possessing
//  "nice" intersection properties with the join.
//
//  Shades are used during the local search phase of the
//  optimizer to determine globally optimal cost
//  and/or cardinality decreasing gradients in
//  the search space.
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#pragma once

#include <vector>
#include <unordered_map>

class Rectangle;

struct Shade {
  Rectangle* rec1;
  Rectangle* rec2;
  Rectangle* join;

  //-----------------------------------
  // Set of rectangles which lie
  // completely inside join
  //-----------------------------------
  std::vector<Rectangle*> envelope;

  //------------------------------------
  // Set of rectangles which are
  // bisected into two sub-rectangles by the join,
  // represented as a map from a rectangle to its
  // shaded portion
  //------------------------------------
  std::unordered_map<Rectangle*, Rectangle*> penumbra;

  Shade(Rectangle* r1, Rectangle* r2, Rectangle* join);

  [[nodiscard]] int penalty() const;
  bool operator< (const Shade &other) const;
};
