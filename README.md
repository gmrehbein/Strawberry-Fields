Strawberry-Fields
=================

Solution to Google-ITA's Strawberry Fields problem:

http://www.itasoftware.com/careers/puzzle_archive.html


Problem: 
Strawberries are growing in a rectangular field of length and width at most 50. You want to build greenhouses
to enclose the strawberries. Greenhouses are axis-aligned with the field (i.e. not diagonal), and may not overlap.
The cost of each greenhouse is $10 plus $1 per unit are covered. Write a program that chooses the best number
of greenhouses to build, and their locations, so as to enclose all the strawberries as cheaply as possible.
Input is a an integer denoting the maximum number of greenhouses to consider and a matrix representation of the field,
in which the '@' symbol represents a strawberry. Output is the original matrix with letters used to represent greenhouses
along with the covering's total cost.

Example:
See strawberries.txt and optimal_covering.txt in this repository for an example input-output pair.


Solution:
The algorithm is fast two-phase approximation algorithm based on greedy matching followed by local search
and is implemented in C++ with extensive use of STL and Boost.

To compile and run, simply unpack the tar bundle, cd into the Strawberry Fields directory, and type 'make' at the command line. The Makefile assumes a reasonably up to date C++ compiler (no use of C++11 features was made) as well as a Boost installation in the canonical search path. The executable is 'strawberryfields' and is invoked as follows:

Usage: strawberryfields [options]
Options:
  -h [ --help ]                                                     show help message
  -f  [ --file ] arg (=strawberries.txt)                 input file
  -o [ --output ] arg (=optimal_covering.txt)   output file


Default arguments are shown in parentheses. I have deliberately kept the complexity of both the build and the run-time options to a minimum. As a benchmark, optimization time on a 3.33Ghz Linux machine for a 50X50 strawberry field  is ~2 seconds.

The algorithm and its implementation are described in more detail below:

Algorithm

0. main() handles argument processing, instantiates the optimizer, reads in the strawberry fields and cardinality constraints defined in the input file, and runs the optimizer on each one in turn

1. Optimizer::generateRectangles() - for an m X n strawberry field, there are C(mn+1,2) - C(m,2)C(n,2) distinct rectangles where C(k,2) is the binomial coefficient enumerating k objects taken 2 at a time. The weight of a rectangle is how many strawberries it covers. We generate the poset of all rectangles along chains (i.e. totally ordered subsets) R_1 < R_2 < ..... < R_m where '<' is the subset relation, discarding those rectangles R_k for which weight(R_k) == weight(R_k-1). The resulting set of rectangles is sorted in ascending weight-to-cost ratio. 

2. Optimizer::greedyMatch() and Optimizer::findNextRectangle() implement a variant of the greedy set cover heuristic, outputting a set of disjoint rectangles that cover all strawberries in the field.

3. Optimizer::localSearch() recursively searches for joins (i.e. convex 2-combinations) among the rectangles in the greedy result set that are globally cost-decreasing and cardinality non-increasing. The search continues while the global cost gradient is negative or we are above the cardinality constraint on the maximum number of rectangles. If there are no negative cost gradients and we are still in excess of the cardinality constraint, the search continues with the least penalizing joins until the cardinality constraint is met.

4. The optimized covering is labeled and outputted to the file specified.

Implementation:

1070 lines of C++ code

main.cc - main() driver handles argument processing and input of strawberry field, instantiates and runs the optimizer

global.h/cc - monostate repository containing data structures for the strawberry field, input and output file pathnames, and the pool interface for creating and destroying rectangles

optimizer.h/cc - implements the optimizing pipeline and prints the result to the output file

rectangle.h/cc - fundamental lightweight object created and manipulated by the optimizer; rectangles are completely specified by 2 pairs of integers and all set operations on the span of a rectangle are implemented using boost::dynamic_bitset<>. The span is computed lazily and all rectangles are created using Global::rectanglePool, memory for which is freed at the end each optimizer run.

shade.h/cc - fundamental objects used to determine globally optimal cost and/or cardinality decreasing gradients during the optimizer's localSearch() phase. Shades consist of two rectangles, their join, two sets of rectangles from the result set (the envelope and penumbra) possessing "nice" intersection properties with the join, together with ordinal and gradient functions


Boost Dependencies:

The following Boost modules are used. The only link-time dependency is program_options, the rest are header-only:

dynamic_bitset
foreach
iterator_range
program_options
singleton_pool
tuple
utility
unordered_map


TO-DO:

Remove sort bias at the beginning of greedy match phase by examining the 8-fold symmetry of a field under the action of the dihedral group D4. Assign each symmetry a thread to run the optimizer pipeline and compare the results at the end of their runs. Transform the best result under its group inverse to recover a potentially more optimal solution than extracted from an untransformed field alone. 

