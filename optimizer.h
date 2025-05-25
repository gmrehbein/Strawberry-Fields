// -----------------------------------------------------------
//  File: optimizer.h
//  Author: Gregory Rehbein (Modernized to C++23)
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#pragma once

#include <cstddef>
#include <list>
#include <vector>
#include <boost/dynamic_bitset.hpp>

// Forward declaration
class Rectangle;

class Optimizer
{
public:
  // Rule of 5 (modern C++ best practice)
  Optimizer() = default;
  ~Optimizer() = default;
  Optimizer(const Optimizer&) = delete;
  Optimizer& operator=(const Optimizer&) = delete;
  Optimizer(Optimizer&&) = default;
  Optimizer& operator=(Optimizer&&) = default;

  //--------------------------------
  // Executes the optimizer.
  // Writes output to the file pathname
  // contained in Global::out_file
  //--------------------------------
  int run();

  //---------------------------------
  // Sets the cardinality constraint
  // on the maximum number of Rectangles
  // to consider
  //---------------------------------
  void set_max_rectangles(int max_rectangles) noexcept;

private:
  void generate_rectangles();
  void greedy_match();
  void local_search();
  void compute_convex_hull();
  void label();
  void output() const;
  void assert_disjoint() const;
  void reset();

  [[nodiscard]] Rectangle* find_next_rectangle();
  [[nodiscard]] Rectangle* join_rectangles(const Rectangle* r1, const Rectangle* r2);

  std::size_t max_rectangles_{0};
  std::vector<Rectangle*> rectangles_;
  std::list<Rectangle*> result_;
  boost::dynamic_bitset<> covering_;
};
