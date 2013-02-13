// -----------------------------------------------------------
//  File: global.cc
//  Author: Gregory Rehbein
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

// Self
#include "global.h"

using std::string;
using std::vector;
using std::set;
using std::pair;

string Global::inFile;
string Global::outFile;
vector<vector<int> > Global::field;
set<pair<int,int> > Global::strawberries;

size_t Global::numRows = 0;
size_t Global::numColumns = 0;

size_t Global::weightOfRectangle
(int topLeftRow, int topLeftColumn, int bottomRightRow, int bottomRightColumn)
{
  size_t weight = 0;
  for (int i = topLeftRow; i <= bottomRightRow; ++i) {
    for (int j = topLeftColumn; j <= bottomRightColumn; ++j) {
      // optimization: use unchecked [] access
      weight += Global::field[i][j];
    }
  }
  return weight;
}
