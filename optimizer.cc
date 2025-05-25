// -----------------------------------------------------------
//  File: optimizer.cpp
//  Author: Gregory Rehbein (Modernized to C++23)
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#include "optimizer.h"

// C standard library
#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>

// C++ standard library
#include <algorithm>
#include <array>
#include <chrono>
#include <concepts>
#include <format>
#include <fstream>
#include <iostream>
#include <print>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

// Third party
#include <boost/dynamic_bitset.hpp>

#include "global.h"
#include "rectangle.h"
#include "shade.h"

using namespace std::chrono;

namespace
{

constexpr std::array<char, 52> alphabet{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

// For a strawberry field of m rows and n columns,
// returns the total number of rectangles that may be generated from it
constexpr std::size_t max_number_of_rectangles(std::size_t m, std::size_t n) noexcept
{
  return ((m * n + 1) * (m * n) / 2 - (m * (m - 1)) * (n * (n - 1)) / 4);
}

// Ordinal type encoding the effect that taking the join of 2 rectangles
enum class IntersectionType : int {
  kVoid = -2,
  kDecreasing = -1,
  kNonIncreasing = 0,
  kIncreasing = 1
};

// For a given triple of rectangles {R1, R2, R3} in the result set,
// the Slice of R3 with respect to the join R1 V R2 is the set R3\(R1 V R2)
struct Slice {
  Rectangle* original{nullptr};
  IntersectionType intersection{IntersectionType::kVoid};
  int top_left_row{-1};
  int top_left_column{-1};
  int bottom_right_row{-1};
  int bottom_right_column{-1};

  explicit constexpr Slice(Rectangle* r) noexcept : original(r) {}

  constexpr auto operator<=>(const Slice& other) const noexcept
  {
    return intersection <=> other.intersection;
  }
};

using Strawberry = std::pair<int, int>;

// Determine the intersection type of *r3 with respect to *join
void determine_intersection_type(const Rectangle* r3, const Rectangle* join, Slice* s)
{
  if (!r3->intersects(join)) {
    s->intersection = IntersectionType::kVoid;
    return;
  }

  if (r3->is_subset_of(join)) {
    s->intersection = IntersectionType::kDecreasing;
    return;
  }

  auto left_over = r3->span() - join->span();
  assert(left_over.any());

  boost::dynamic_bitset<> test(r3->span().size());
  std::set<std::size_t> rows, columns;

  auto pos = left_over.find_first();
  auto top_left_column = pos % Global::num_columns;
  auto top_left_row = (pos - top_left_column) / Global::num_columns;

  // Collect all positions and track the last valid position
  auto last_pos = pos;
  while (pos != boost::dynamic_bitset<>::npos) {
    const auto col = pos % Global::num_columns;
    const auto row = (pos - col) / Global::num_columns;
    rows.insert(row);
    columns.insert(col);
    last_pos = pos;  // Keep track of the last valid position
    pos = left_over.find_next(pos);
  }

  // Use the last valid position for bottom-right coordinates
  const auto bottom_right_column = last_pos % Global::num_columns;
  const auto bottom_right_row = (last_pos - bottom_right_column) / Global::num_columns;

  // Set test bits for rectangle area
  for (std::size_t i = top_left_row; i <= bottom_right_row; ++i) {
    for (std::size_t j = top_left_column; j <= bottom_right_column; ++j) {
      const auto bit_pos = i * Global::num_columns + j;
      if (bit_pos < test.size()) {  // Add bounds check
        test.set(bit_pos);
      }
    }
  }

  const bool is_valid_bounds = (top_left_row == *rows.begin() &&
                                top_left_column == *columns.begin() &&
                                bottom_right_row == *rows.rbegin() &&
                                bottom_right_column == *columns.rbegin());

  const bool is_rectangle = is_valid_bounds && (test == left_over);

  if (is_rectangle) {
    s->intersection = IntersectionType::kNonIncreasing;
    s->top_left_row = static_cast<int>(top_left_row);
    s->top_left_column = static_cast<int>(top_left_column);
    s->bottom_right_row = static_cast<int>(bottom_right_row);
    s->bottom_right_column = static_cast<int>(bottom_right_column);
  } else {
    s->intersection = IntersectionType::kIncreasing;
  }
}

} // anonymous namespace

int Optimizer::run()
{
  const auto start_time = steady_clock::now();

  if (max_rectangles_ > 1) {
    generate_rectangles();
    greedy_match();
    local_search();
  } else {
    compute_convex_hull();
  }

  label();

  const auto end_time = steady_clock::now();
  const auto elapsed = duration<double>(end_time - start_time);

  std::println("optimized {} X {} field of {} strawberries in {:.6f} seconds",
               Global::num_rows, Global::num_columns,
               Global::strawberries.size(), elapsed.count());

  output();

  int total_cost = 0;
  for (const auto* rect : result_) {
    total_cost += static_cast<int>(rect->cost());
  }

  reset();
  return total_cost;
}

void Optimizer::reset()
{
  result_.clear();
  Global::purge_memory();
  max_rectangles_ = 0;
}

void Optimizer::set_max_rectangles(int m) noexcept
{
  max_rectangles_ = static_cast<std::size_t>(m);
}

void Optimizer::generate_rectangles()
{
  const int M = static_cast<int>(Global::num_rows);
  const int N = static_cast<int>(Global::num_columns);

  rectangles_.reserve(max_number_of_rectangles(M, N));

  for (int row = 0; row < M; ++row) {
    for (int col = 0; col < N; ++col) {
      for (int right = col; right < N; ++right) {
        std::size_t weight = 0;
        for (int down = row; down < M; ++down) {
          const auto w = Global::weight_of_rectangle(row, col, down, right);
          if (w > weight) {
            auto* r = new(Global::malloc())
            Rectangle(row, col, down, right, w);
            rectangles_.push_back(r);
            weight = w;
          }
        }
      }
    }
  }

  std::ranges::sort(rectangles_, Rectangle::better);
}

void Optimizer::greedy_match()
{
  assert(covering_.none());

  const auto total_bits = Global::num_columns * Global::num_rows;
  boost::dynamic_bitset<> unmatched_strawberries(total_bits);

  for (const auto& [first, second] : Global::strawberries) {
    const auto bit_pos = Global::num_columns * static_cast<std::size_t>(first) +
                         static_cast<std::size_t>(second);

    if (bit_pos >= total_bits) {
      continue;
    }
    unmatched_strawberries.set(bit_pos);
  }

  covering_.resize(total_bits);

  while (unmatched_strawberries.any()) {
    auto* r = find_next_rectangle();
    covering_ |= r->span();
    result_.push_back(r);
    unmatched_strawberries -= covering_;
  }

  rectangles_.clear();
  covering_.clear();
}

Rectangle* Optimizer::find_next_rectangle()
{
  auto* r = rectangles_.back();
  r->make_span(); // lazy initialization

  if (covering_.any()) {
    boost::dynamic_bitset<> meet;
    do {
      r = rectangles_.back();
      r->make_span();
      meet = covering_ & r->span();
      rectangles_.pop_back();
    } while (meet.any() && !rectangles_.empty());
  }

  return r;
}

Rectangle* Optimizer::join_rectangles(const Rectangle* r1, const Rectangle* r2)
{
  assert(!r1->intersects(r2)); // after greedy match, rectangles are disjoint

  const auto top_left_row = std::min(r1->top_left_row(), r2->top_left_row());
  const auto top_left_column = std::min(r1->top_left_column(), r2->top_left_column());
  const auto bottom_right_row = std::max(r1->bottom_right_row(), r2->bottom_right_row());
  const auto bottom_right_column = std::max(r1->bottom_right_column(), r2->bottom_right_column());

  auto* r = new(Global::malloc())
  Rectangle(top_left_row, top_left_column,
            bottom_right_row, bottom_right_column);
  r->make_span();
  return r;
}

void Optimizer::assert_disjoint() const
{
  for (auto it1 = result_.begin(); it1 != result_.end(); ++it1) {
    for (auto it2 = std::next(it1); it2 != result_.end(); ++it2) {
      assert(!(*it1)->intersects(*it2));
    }
  }
}

void Optimizer::local_search()
{
  if (result_.size() < 2) return;

  std::vector<Shade> shades;

  // Generate all pairs manually
  for (auto it1 = result_.begin(); it1 != result_.end(); ++it1) {
    for (auto it2 = std::next(it1); it2 != result_.end(); ++it2) {
      auto* r1 = *it1;
      auto* r2 = *it2;
      auto* join = join_rectangles(r1, r2);
      Shade shade(r1, r2, join);

      // Build difference set (all rectangles except r1 and r2)
      std::vector<Rectangle*> difference_set;
      for (auto* r : result_) {
        if (r != r1 && r != r2) {
          difference_set.push_back(r);
        }
      }

      std::vector<Slice> slices;
      for (auto* r3 : difference_set) {
        Slice s(r3);
        determine_intersection_type(r3, join, &s);
        if (s.intersection != IntersectionType::kVoid) {
          slices.push_back(s);
        }
      }

      if (slices.empty()) {
        shades.push_back(std::move(shade));
      } else {
        std::sort(slices.begin(), slices.end());
        if (slices.back().intersection != IntersectionType::kIncreasing) {
          for (const auto& s : slices) {
            if (s.intersection == IntersectionType::kDecreasing) {
              shade.envelope.push_back(s.original);
            } else {
              auto* r = new(Global::malloc())
              Rectangle(s.top_left_row, s.top_left_column,
                        s.bottom_right_row, s.bottom_right_column);
              r->make_span();
              shade.penumbra[s.original] = r;
            }
          }
          shades.push_back(std::move(shade));
        }
      }
    }
  }

  std::sort(shades.begin(), shades.end());

  if (!shades.empty()) {
    const auto& best = shades.front();
    if (best.penalty() <= 0 || result_.size() > max_rectangles_) {
      std::erase(result_, best.rec1);
      std::erase(result_, best.rec2);
      result_.push_back(best.join);

      for (auto* r : best.envelope) {
        std::erase(result_, r);
      }

      for (const auto& [original, slice] : best.penumbra) {
        std::ranges::replace(result_, original, slice);
      }

      local_search(); // recursive call
    }
  }
}

void Optimizer::compute_convex_hull()
{
  boost::dynamic_bitset<> strawberries(Global::num_columns * Global::num_rows);

  for (const auto& [first, second] : Global::strawberries) {
    strawberries.set(Global::num_columns * static_cast<std::size_t>(first) +
                     static_cast<std::size_t>(second));
  }

  std::set<std::size_t> rows, columns;

  auto pos = strawberries.find_first();
  while (pos != boost::dynamic_bitset<>::npos) {
    const auto col = pos % Global::num_columns;
    const auto row = (pos - col) / Global::num_columns;
    rows.insert(row);
    columns.insert(col);
    pos = strawberries.find_next(pos);
  }

  const auto top_left_row = *rows.begin();
  const auto top_left_column = *columns.begin();
  const auto bottom_right_row = *rows.rbegin();
  const auto bottom_right_column = *columns.rbegin();

  auto* r = new(Global::malloc())
  Rectangle(static_cast<int>(top_left_row), static_cast<int>(top_left_column),
            static_cast<int>(bottom_right_row), static_cast<int>(bottom_right_column));
  r->make_span();
  result_.push_back(r);
}

void Optimizer::label()
{
  result_.sort(Rectangle::better);
  result_.reverse();

  std::size_t index = 0;
  for (auto* rect : result_) {
    const char label = (index < alphabet.size()) ? alphabet[index] : '0';
    rect->set_label(label);
    ++index;
  }
}

void Optimizer::output() const
{
  std::vector<std::vector<char>> output(Global::num_rows,
                                        std::vector<char>(Global::num_columns, '.'));

  std::ofstream out_file(Global::out_file, std::ios_base::out | std::ios_base::app);

  int cost = 0;
  for (const auto* r : result_) {
    cost += static_cast<int>(r->cost());
    auto pos = r->span().find_first();
    while (pos != boost::dynamic_bitset<>::npos) {
      const auto col = pos % Global::num_columns;
      const auto row = (pos - col) / Global::num_columns;
      output[row][col] = r->label();
      pos = r->span().find_next(pos);
    }
  }

  out_file << std::format("Cardinality:{}\nCost:{}\n", result_.size(), cost);
  out_file << std::string(Global::num_columns, '=') << '\n';

  for (const auto& row : output) {
    for (char c : row) {
      out_file << c;
    }
    out_file << '\n';
  }
  out_file << '\n';
}
