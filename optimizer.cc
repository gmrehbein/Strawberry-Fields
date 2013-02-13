// -----------------------------------------------------------
//  File: optimizer.cc
//  Author: Gregory Rehbein
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

//  Self
#include "optimizer.h"

//  C
#include <ctime>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <unistd.h>

//  C++
#include <algorithm>
#include <set>
#include <iostream>
#include <fstream>

//  Boost
#include <boost/dynamic_bitset.hpp>
#include <boost/foreach.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/tuple/tuple.hpp>

#include "global.h"
#include "rectangle.h"
#include "shade.h"

using std::pair;
using std::list;
using std::vector;
using std::set;
using std::sort;
using std::min;
using std::max;
using std::fill;
using std::replace;
using std::ofstream;
using std::ios_base;
using boost::dynamic_bitset;
using boost::iterator_range;

#define foreach BOOST_FOREACH

namespace
{
char alphabet[] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
  'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
  'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
  'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
  's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};

//------------------------------------------------
// For a strawberry field of m rows and n columns,
// returns the total number of rectangles that may be
// generated from it
//-----------------------------------------------
inline size_t maxNumberOfRectangles(size_t m, size_t n)
{
  return ((m*n+1)*(m*n)/2 - (m*(m-1))*(n*(n-1))/4);
}

//-----------------------------------------------------
// Ordinal type encoding the effect that taking the
// join of 2 rectangles {R1, R2} from the result set R
// has on the difference set R' = R\{R1,R2}. An join can either
// not intersect R' (kVoid), only intersect R' in the envelope
// (kDecreasing), intersect R' in the penumbra (kNonIncreasing),
// or intersect a rectangle in R' such that resulting fragment
// is constructed out of more than one rectangle (kIncreasing)
// (e.g. a notch cut out of a corner).
//-----------------------------------------------------
enum IntersectionType {
  kVoid = -2,
  kDecreasing = -1,
  kNonIncreasing = 0,
  kIncreasing = 1
};

//------------------------------------------------------
// For a given triple of rectangles {R1, R2, R3}
// in the result set, the Slice of R3 with respect
// to the join R1 V R2 is the set R3\(R1 V R2).
// The IntersectionType corresponds to the cases:
//
// kVoid          <----> R3\(R1VR2) == R3
// kDecreasing    <----> R3 < R1 V R2
// kNonIncreasing <----> R3\(R1 V R2) is a rectangle
// kIncreasing    <----> R3\(R1 V R2) is NOT a rectangle
//
// The localSearch() phase of the optimizer rules out joins
// which increase the cardinality of the result set (i.e.
// those Slices with IntersectionType kIncreasing).
//-------------------------------------------------------
struct Slice {
  Rectangle* original;  // original rectangle
  IntersectionType intersection;
  int topLeftRow;
  int topLeftColumn;
  int bottomRightRow;
  int bottomRightColumn;
  explicit Slice(Rectangle* r)
    : original(r) {
    topLeftRow = topLeftColumn = bottomRightRow = bottomRightColumn = -1;
  }
  inline bool operator<(const Slice& other) const {
    return intersection < other.intersection;
  }
};

//--------------------------------------------------------
// For given rectangles *r3 and *join, and Slice *s with
// s->original == r3, determine the intersection type of
// *r3 with respect to *join, in particular whether
// the intersection is or is not a rectangle.
//
// Note: attempting to accelerate the localSearch() phase by
// memoizing the intersection types for given triples of
// rectangles {R1, R2, R3} in a lookup table actually slows
// down the computation. For the recursion depths that we
// see in practice, recomputing the intersection type each
// recursion is faster than table look-up
//---------------------------------------------------------
void determineIntersectionType(const Rectangle* r3, const Rectangle* join, Slice* s)
{
  if (!r3->intersects(join)) {
    s->intersection = kVoid;
  } else if (r3->isSubsetOf(join)) {
    s->intersection = kDecreasing;
  } else {
    dynamic_bitset<> left_over = r3->span() - join->span();

    assert(left_over.any());

    dynamic_bitset<> test(r3->span());
    test.reset();

    set<size_t> rows;
    set<size_t> columns;

    size_t pos = left_over.find_first();
    size_t topLeftColumn = pos % Global::numColumns;
    size_t topLeftRow = (pos - topLeftColumn)/Global::numColumns;

    size_t next_index = pos;
    size_t col, row;
    while (next_index != dynamic_bitset<>::npos) {
      pos = next_index;
      col = pos % Global::numColumns;
      row = (pos - col)/Global::numColumns;
      rows.insert(row);
      columns.insert(col);
      next_index = left_over.find_next(pos);
    }

    size_t bottomRightColumn = pos % Global::numColumns;
    size_t bottomRightRow = (pos - bottomRightColumn)/Global::numColumns;

    for (size_t i = topLeftRow; i <= bottomRightRow; ++i)
      for (size_t j = topLeftColumn; j <= bottomRightColumn; ++j)
        test.set(i*Global::numColumns + j);

    // condition 1: topLeftRow and topLeftColumn must be
    // the min among all rows and columns, resp.
    bool c1 = (topLeftRow == *(rows.begin()) && topLeftColumn == *(columns.begin()));

    // condition 2: bottomRightRow and bottomRightColumn must be
    // the max among all rows and columns, resp.
    bool c2 = (bottomRightRow == *(rows.rbegin()) && bottomRightColumn == *(columns.rbegin()));

    // condition 3: rectangle must have no holes
    bool c3 = (test == left_over);

    bool isRectangle = c1 && c2 && c3;

    if (isRectangle) {
      s->intersection = kNonIncreasing;
      // caller will use this to generate new Rectangle
      s->topLeftRow = topLeftRow;
      s->topLeftColumn = topLeftColumn;
      s->bottomRightRow = bottomRightRow;
      s->bottomRightColumn = bottomRightColumn;
    } else {
      s->intersection = kIncreasing;
    }
  }  // end case of non cardinality-decreasing intersections
}

typedef list<Rectangle*>::iterator ListIterator;
typedef pair<int, int> strawberry;

}  // end anon namespace

Optimizer::Optimizer()
  :m_maxRectangles(0)
{
}

Optimizer::~Optimizer()
{
  m_rectangles.clear();
  m_covering.clear();
}

int Optimizer::run()
{
  clock_t start_time, end_time;
  double elapsed_time_seconds;

  start_time = clock();

  if (m_maxRectangles > 1) {
    generateRectangles();
    greedyMatch();
    localSearch();
  } else {
    computeConvexHull();
  }
  label();
  end_time = clock();
  elapsed_time_seconds = (static_cast<double>(end_time - start_time))
                         / CLOCKS_PER_SEC;
  printf("optimized %zu X %zu field of %zu strawberries in %.6f seconds\n",
         Global::numRows, Global::numColumns,
         Global::strawberries.size(), elapsed_time_seconds);
  output();
  int totalCost = 0;
  foreach(Rectangle* r, m_result) totalCost += r->cost();
  reset();
  return totalCost;
}

void Optimizer::reset()
{
  m_result.clear();
  Global::rectanglePool.purge_memory();
  m_maxRectangles = 0;
}

void Optimizer::setMaxRectangles(int m)
{
  m_maxRectangles = m;
}

//-----------------------------------------------
// First phase of the optimizer pipeline:
// For an M X N strawberry field, generate the
// poset of rectangles along chains:
// R_1 < ...... < R_N-1 < R_N, where < is the
// subset relation.
//
// Discard those rectangles R_k for which
// weight(R_k) == weight (R_k-1).
// Sort the resulting set of rectangles in order
// of increasing weight-to-cost ratio.
//-----------------------------------------------
void Optimizer::generateRectangles()
{
  const int M = Global::numRows;
  const int N = Global::numColumns;

  m_rectangles.reserve(maxNumberOfRectangles(M, N));

  for (int row = 0; row < M; ++row) {
    for (int col = 0; col < N; ++col) {
      for (int right = col; right < N; ++right) {
        //  begin generating chain
        size_t weight = 0;
        for (int down = row; down < M; ++down) {
          size_t w = Global::weightOfRectangle(row, col, down, right);
          if (w > weight) {
            Rectangle* r =  new(Global::rectanglePool.malloc())
            Rectangle(row, col, down, right, w);
            m_rectangles.push_back(r);
            weight = w;
          }
        }
      }
    }
  }
  sort(m_rectangles.begin(), m_rectangles.end(), Rectangle::better);
}

//------------------------------------------------------
// greedyMatch() and findNextRectangle() constitute
// the second phase of the optimizer pipeline. Using
// a variant of the greedy set cover heuristic,
// greedyMatch() outputs a disjoint collection of
// rectangles that covers all strawberries in the field.
//------------------------------------------------------
void Optimizer::greedyMatch()
{
//  Pre-conditions
  assert(m_covering.none());

//  define bitmap to where the strawberries are
  dynamic_bitset<> unmatchedStrawberries(Global::numColumns * Global::numRows);
  foreach(const strawberry& s, Global::strawberries) {
    unmatchedStrawberries.set(Global::numColumns*s.first + s.second);
  }
  m_covering.resize(Global::numColumns * Global::numRows);
  dynamic_bitset<> allStrawberries(unmatchedStrawberries);

  do {
    Rectangle* r = findNextRectangle();
    m_covering |= r->span();

    //  promote rectangle from candidate to semi-finalist
    m_result.push_back(r);

    unmatchedStrawberries -= m_covering;
  } while (unmatchedStrawberries.any());
  m_rectangles.clear();
  m_covering.clear();
}

Rectangle* Optimizer::findNextRectangle()
{
  Rectangle* r = m_rectangles.back();
  // lazy initialization
  r->makeSpan();

  if (m_covering.any()) {
    dynamic_bitset<> meet;
    do {
      r = m_rectangles.back();
      // lazy initialization
      r->makeSpan();
      meet = m_covering & r->span();
      m_rectangles.pop_back();
    } while (meet.any() && m_rectangles.size() > 0);
  }
  return r;
}

//--------------------------------------
// Returns a new rectangle that is the
// rectangular hull of *r1 and *r2
//--------------------------------------
Rectangle* Optimizer::joinRectangles(const Rectangle* r1, const Rectangle* r2)
{
//  after greedy match, rectangles are all disjoint
  assert(!r1->intersects(r2));

  int topLeftRow = min(r1->topLeftRow(), r2->topLeftRow());
  int topLeftColumn = min(r1->topLeftColumn(), r2->topLeftColumn());
  int bottomRightRow = max(r1->bottomRightRow(), r2->bottomRightRow());
  int bottomRightColumn = max(r1->bottomRightColumn(), r2->bottomRightColumn());
  Rectangle* r =  new(Global::rectanglePool.malloc())
  Rectangle(topLeftRow, topLeftColumn,
            bottomRightRow, bottomRightColumn);
  // lazy initialization
  r->makeSpan();
  return r;
}

void Optimizer::assertDisjoint()
{
  ListIterator end = m_result.end();
  for (ListIterator i = m_result.begin(); i != end; ++i)
    for (ListIterator j = i; ++j != end; /**/)
      assert(!((*i)->intersects(*j)));
}

//----------------------------------------------------------------
// Third and final stage of the optimizer pipeline.
// localSearch() recursively searches for joins among the rectangles
// in the result set that are globally cost-decreasing
// and cardinality non-increasing. The search continues
// while the global cost gradient is negative or we are above
// the maximum number of rectangles constraint. If there are no
// negative cost gradients and we are still above m_maxRectangles,
// the search continues with the least penalizing joins until
// the covering cardinality constraint is met.
//----------------------------------------------------------------
void Optimizer::localSearch()
{
  if (m_result.size() < 2 )
    return;

  list<Shade> shades;
  ListIterator end = m_result.end();
  for (ListIterator i = m_result.begin(); i != end; ++i) {
    for (ListIterator j = i; ++j != end; /**/ ) {
      Rectangle* r1 = *i;
      Rectangle* r2 = *j;
      Rectangle* join = joinRectangles(r1, r2);
      Shade shade(r1, r2, join);
      ListIterator k = i;
      ListIterator m = j;
      vector<Rectangle*> difference_set;

      foreach(Rectangle* r, iterator_range<ListIterator> (m_result.begin(), k))
      difference_set.push_back(r);
      foreach(Rectangle* r, iterator_range<ListIterator> (++k, m))
      difference_set.push_back(r);
      foreach(Rectangle* r, iterator_range<ListIterator> (++m, m_result.end()))
      difference_set.push_back(r);

      vector<Slice> slices;
      foreach(Rectangle* r3, difference_set) {
        Slice s(r3);
        determineIntersectionType(r3, join, &s);
        if (kVoid != s.intersection)
          slices.push_back(s);
      }

      if (slices.empty()) {
        // join did not intersect any rectangle in result set
        shades.push_back(shade);
      } else {
        sort(slices.begin(), slices.end());
        if (slices.back().intersection != kIncreasing) {
          foreach(Slice s, slices) {
            if (s.intersection == kDecreasing) {
              shade.envelope.push_back(s.original);
            } else {
              Rectangle* r =
                new(Global::rectanglePool.malloc())
              Rectangle(s.topLeftRow, s.topLeftColumn,
                        s.bottomRightRow, s.bottomRightColumn);
              r->makeSpan();
              shade.penumbra[s.original] = r;
            }
          }
          shades.push_back(shade);
        }
      }  // end case of non-empty slices
    }
  }  // end iteration over 2-combinations in result set

  shades.sort();
  const Shade& best = shades.front();
  if (best.penalty() <= 0 || m_result.size() > m_maxRectangles) {
    m_result.remove(best.m_r1);
    m_result.remove(best.m_r2);
    m_result.push_back(best.m_join);
    foreach(Rectangle* r, best.envelope) {
      m_result.remove(r);
    }

    Rectangle *original, *slice;
    foreach(boost::tie(original, slice), best.penumbra) {
      replace(m_result.begin(), m_result.end(), original, slice);
    }

    localSearch();
  }
}

//---------------------------------------------------------
// For the special case where the covering cardinality
// constraint is 1, short cut the optimization
// by computing the rectangular hull of the top-left-most
// and bottom-right-most strawberries
//--------------------------------------------------------
void Optimizer::computeConvexHull()
{
//  define bitmap to where the strawberries are
  dynamic_bitset<> strawberries(Global::numColumns * Global::numRows);
  foreach(strawberry s, Global::strawberries) {
    strawberries.set(Global::numColumns*s.first + s.second);
  }

  set<size_t> rows;
  set<size_t> columns;
  size_t col, row;
  size_t pos = strawberries.find_first();
  do {
    col = pos % Global::numColumns;
    row = (pos - col)/Global::numColumns;
    rows.insert(row);
    columns.insert(col);
    pos = strawberries.find_next(pos);
  } while (pos != dynamic_bitset<>::npos);

  size_t topLeftRow = *(rows.begin());
  size_t topLeftColumn = *(columns.begin());
  size_t bottomRightRow = *(rows.rbegin());
  size_t bottomRightColumn = *(columns.rbegin());

  Rectangle* r = new(Global::rectanglePool.malloc())
  Rectangle(topLeftRow, topLeftColumn, bottomRightRow, bottomRightColumn);
  r->makeSpan();
  m_result.push_back(r);
}

void Optimizer::label()
{
  m_result.sort(Rectangle::better);
  m_result.reverse();
  int index = 0;
  foreach(Rectangle* r, m_result) {
    index < 52 ? r->setLabel(alphabet[index]) : r->setLabel('0');
    ++index;
  }
}

void Optimizer::output()
{
  char output[Global::numRows][Global::numColumns];
  fill(&output[0][0], &output[0][0] + sizeof(output), '.');
  ofstream outFile(Global::outFile.c_str(), ios_base::out | ios_base::app);
  size_t row, col;
  row = col = 0;
  int cost = 0;
  foreach(Rectangle* r, m_result) {
    cost += r->cost();
    size_t pos = r->span().find_first();
    while (pos != dynamic_bitset<>::npos) {
      col = pos % Global::numColumns;
      row = (pos - col)/Global::numColumns;
      output[row][col] = r->label();
      pos = r->span().find_next(pos);
    }
  }
  outFile << "Cardinality:" << m_result.size() << "\n"
          << "Cost:" << cost << "\n";
  for (size_t j = 0; j < Global::numColumns; ++j) {
    outFile << "=";
  }
  outFile << "\n";
  for (row = 0; row < Global::numRows; ++row) {
    for (col = 0; col < Global::numColumns; ++col) {
      outFile << output[row][col];
    }
    outFile << "\n";
  }
  outFile << "\n";
  outFile.close();
}

#undef foreach
