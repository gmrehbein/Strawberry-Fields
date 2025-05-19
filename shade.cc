// -----------------------------------------------------------
//  File: shade.cc
//  Author: Gregory Rehbein
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

// Self
#include "shade.h"

// C
#include <cassert>

// Local
#include "rectangle.h"

Shade::Shade(Rectangle* r1, Rectangle* r2, Rectangle* join)
: rec1(r1), rec2(r2), join(join)
{
  assert(rec1 != nullptr);
  assert(rec2 != nullptr);
  assert(join != nullptr);
}

//--------------------------------------------------
// Gradient function for the Shade class.
// If penalty() < 0, then the join together with
// its penumbra is cheaper to include in the Optimizer
// result set than its constituent components.
//--------------------------------------------------
int Shade::penalty() const
{
  int envelopeCost = 0;
  int penumbraCost = 0;

  for (auto rec : envelope) {
    assert(rec != nullptr);
    envelopeCost += rec->cost();
  }

  for (auto [original, slice] : penumbra) {
    penumbraCost += original->area() - slice->area();
  }

  int penalty = join->cost() - (rec1->cost() + rec2->cost() + envelopeCost + penumbraCost);
  return penalty;
}

//----------------------------------------------
// Ordinal function for the Shade class. If
// two Shades have the same penalty, choose
// the Shade that encompasses less rectangles
// in its envelope since that leaves more choices
// for the optimizer's localSearch() to test
//----------------------------------------------
bool Shade::operator< (const Shade &other) const
{
  return penalty() == other.penalty() ?
         envelope.size() < other.envelope.size() : penalty() < other.penalty();
}
