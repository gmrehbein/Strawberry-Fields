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
#include <cstdio>

// BOOST
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#include "rectangle.h"

#define foreach BOOST_FOREACH

Shade::Shade(Rectangle* r1, Rectangle* r2, Rectangle* join)
{
  assert(r1 != NULL);
  assert(r1 != NULL);
  assert(join != NULL);
  m_r1 = r1;
  m_r2 = r2;
  m_join = join;
}

Shade::~Shade()
{
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
  foreach(Rectangle* r, envelope) {
    assert(r != NULL);
    envelopeCost += r->cost();
  }
  Rectangle *original, *slice;
  foreach(boost::tie(original, slice), penumbra) {
    penumbraCost += original->area() - slice->area();
  }
  int penalty = m_join->cost() - (m_r1->cost() + m_r2->cost() + envelopeCost + penumbraCost);
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

#undef foreach
