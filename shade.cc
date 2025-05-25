// -----------------------------------------------------------
//  File: shade.cc
//  Author: Gregory Rehbein (Modernized to C++23)
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#include "shade.h"

#include <algorithm>
#include <cassert>
#include <ranges>

#include "rectangle.h"

Shade::Shade(Rectangle* r1, Rectangle* r2, Rectangle* join_rect)
  : rec1(r1), rec2(r2), join(join_rect)
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
  // Calculate envelope cost using ranges
  const auto envelope_cost = std::ranges::fold_left(
  envelope | std::views::transform([](const Rectangle* rec) {
    assert(rec != nullptr);
    return rec->cost();
  }),
  0, std::plus<> {});

  // Calculate penumbra cost using ranges
  const auto penumbra_cost = std::ranges::fold_left(
  penumbra | std::views::transform([](const auto& pair) {
    const auto& [original, slice] = pair;
    return original->area() - slice->area();
  }),
  0, std::plus<> {});

  const int penalty_value = join->cost() - (rec1->cost() + rec2->cost() +
                            envelope_cost + penumbra_cost);
  return penalty_value;
}

//----------------------------------------------
// Ordinal function for the Shade class. If
// two Shades have the same penalty, choose
// the Shade that encompasses fewer rectangles
// in its envelope since that leaves more choices
// for the optimizer's local_search() to test
//----------------------------------------------
bool Shade::operator<(const Shade& other) const
{
  const auto this_penalty = penalty();
  const auto other_penalty = other.penalty();

  return this_penalty == other_penalty
         ? envelope.size() < other.envelope.size()
         : this_penalty < other_penalty;
}
