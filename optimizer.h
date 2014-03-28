// -----------------------------------------------------------
//  File: optimizer.h
//  Author: Gregory Rehbein
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <stdint.h>
#include <vector>
#include <list>
#include <boost/utility.hpp>
#include <boost/dynamic_bitset.hpp>

class Rectangle;

class Optimizer : boost::noncopyable
{
public:
  Optimizer();
  ~Optimizer();

  //--------------------------------
  // Executes the optimizer.
  // Writes output to the file pathname
  // contained in Global::outFile
  //--------------------------------
  int run();

  //---------------------------------
  // Sets the cardinality constraint
  // on the maximum number of Rectangles
  // to consider
  //---------------------------------
  void setMaxRectangles(int maxRectangles);

private:
  void generateRectangles();
  void greedyMatch();
  void localSearch();
  void computeConvexHull();
  void label();
  void output();
  void assertDisjoint();
  void reset();
  Rectangle* findNextRectangle();
  Rectangle* joinRectangles(const Rectangle*, const Rectangle*);

  size_t m_maxRectangles;
  std::vector<Rectangle* > m_rectangles;
  std::list<Rectangle*> m_result;
  boost::dynamic_bitset<> m_covering;
};

#endif
