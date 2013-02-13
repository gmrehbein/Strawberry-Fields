// -----------------------------------------------------------
//  File: main.cc
//  Author: Gregory Rehbein
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

// C
#include <cstdlib>

// C++
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Boost
#include <boost/program_options.hpp>

#include "optimizer.h"
#include "global.h"

using std::string;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::make_pair;
using std::vector;
using std::ios_base;

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
// process options
  po::options_description desc("Options");
  try {
    desc.add_options()
    ("help,h", "show help message")
    ("file,f", po::value<string>
     (&Global::inFile)->default_value("strawberries.txt"), "input file")
    ("output,o", po::value<string>
     (&Global::outFile)->default_value("optimal_covering.txt"), "output file");

    po::positional_options_description p;
    p.add("file", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
      cout << "Usage: strawberryfields [options]\n";
      cout << desc;
      return 1;
    }
  }

  catch (std::exception& e) {
    cout << e.what() << "\n";
    cout << "Usage: strawberryfields [options]\n";
    cout << desc;
    return 1;
  }

  Optimizer optimizer;
  int maxRectangles;
  int totalCost = 0;

// input arrays
  ifstream strawberryFile(Global::inFile.c_str());
  char line[52];
  while (strawberryFile.getline(line, 52).good()) {
    static int m = -1;  // row count
    if (strawberryFile.gcount() > 1) {
      if (isdigit(line[0])) {
        maxRectangles = strtol(line, NULL, 10);
        optimizer.setMaxRectangles(maxRectangles);
      } else {
        // read line into field
        vector<int> row;
        ++m;
        int numColumns = strawberryFile.gcount() - 1;
        row.reserve(numColumns);
        for (int n = 0; n < numColumns; ++n) {
          if ('.' == line[n])
            row.push_back(0);
          else if ('@' == line[n]) {
            row.push_back(1);
            Global::strawberries.insert(make_pair(m, n));
          }
        }
        Global::field.push_back(row);
        row.clear();
      }
    } else if (!Global::field.empty()) {
      // we read a newline and are ready to
      // process the strawberry patch
      m = -1;
      Global::numRows = Global::field.size();
      Global::numColumns = Global::field.at(0).size();
      totalCost += optimizer.run();
      Global::field.clear();
      Global::strawberries.clear();
      Global::numRows = Global::numColumns = 0;
    }
  }
  // handle the last field
  if (strawberryFile.eof() && !Global::field.empty()) {
    Global::numRows = Global::field.size();
    Global::numColumns = Global::field.at(0).size();
    totalCost += optimizer.run();
  }
  strawberryFile.close();
  ofstream output(Global::outFile.c_str(), ios_base::out | ios_base::app);
  output << "Total Cost: " << totalCost << "\n";
  output.close();
  return 0;
}
