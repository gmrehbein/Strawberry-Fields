// -----------------------------------------------------------
//  File: main.cc
//  Author: Gregory Rehbein (Modernized to C++23)
//
//  Copyright (C) 2012 Gregory Rehbein <gmrehbein@gmail.com>
// -----------------------------------------------------------

#include <cctype>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector> // NOLINT(misc-include-cleaner)

#include <boost/program_options.hpp>

#include "global.h"
#include "optimizer.h"

namespace po = boost::program_options;

namespace
{
void process_field_line(std::string_view line, int row_index)
{
  std::vector<int> row;
  row.reserve(line.size());

  for (auto col_index : std::views::iota(0UZ, line.size())) {
    const char ch = line[col_index];
    if (ch == '.') {
      row.push_back(0);
    } else if (ch == '@') {
      row.push_back(1);
      Global::strawberries.emplace(row_index, static_cast<int>(col_index));
    }
  }

  Global::field.push_back(std::move(row));
}

void process_strawberry_field(Optimizer& optimizer, int& total_cost)
{
  if (Global::field.empty()) return;

  Global::num_rows = Global::field.size();
  Global::num_columns = Global::field.at(0).size();
  total_cost += optimizer.run();

  // Reset for next field
  Global::field.clear();
  Global::strawberries.clear();
  Global::num_rows = Global::num_columns = 0;
}
}

int main(int argc, char* argv[])
{
  // Process command line options
  po::options_description desc("Options");
  try {
    desc.add_options()
    ("help,h", "show help message")
    ("file,f", po::value<std::string>(&Global::in_file)
     ->default_value("strawberries.txt"), "input file")
    ("output,o", po::value<std::string>(&Global::out_file)
     ->default_value("optimal_covering.txt"), "output file");

    po::positional_options_description p;
    p.add("file", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
              .options(desc)
              .positional(p)
              .run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::println("Usage: strawberryfields [options]");
      std::cout << desc;
      return EXIT_SUCCESS;
    }
  } catch (const std::exception& e) { // NOLINT(bugprone-empty-catch)
    std::println("Error: {}", e.what());
    std::println("Usage: strawberryfields [options]");
    std::cout << desc;
    return EXIT_FAILURE;
  }

  Optimizer optimizer;
  int total_cost = 0;

  // Process input file
  std::ifstream strawberry_file(Global::in_file);
  if (!strawberry_file) {
    std::println("Error: Cannot open input file '{}'", Global::in_file);
    return EXIT_FAILURE;
  }

  std::string line;
  int row_index = -1;

  while (std::getline(strawberry_file, line)) {
    if (line.empty()) {
      // Empty line signals end of current field
      process_strawberry_field(optimizer, total_cost);
      row_index = -1;
      continue;
    }

    if (std::isdigit(line[0])) {
      // First line with digit is max rectangles constraint
      const auto max_rectangles = std::stoi(line);
      optimizer.set_max_rectangles(max_rectangles);
    } else {
      // Field data line
      ++row_index;
      process_field_line(line, row_index);
    }
  }

  // Handle the last field if file doesn't end with empty line
  if (!Global::field.empty()) {
    process_strawberry_field(optimizer, total_cost);
  }

  // Write total cost to output file
  {
    std::ofstream output(Global::out_file, std::ios_base::out | std::ios_base::app);
    output << "Total Cost: " << total_cost << '\n';
  }

  return EXIT_SUCCESS;
}
