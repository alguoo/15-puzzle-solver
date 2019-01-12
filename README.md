# 15 Puzzle Solver
Completed a solver for the [15 Puzzle](https://en.wikipedia.org/wiki/15_puzzle), a common sliding puzzle.
The solver implements Iterative Deepening A* (IDA*) search algorithm to find the minimal number of moves to solve the puzzle, by reducing states of the puzzle as nodes, and edges representing states which are one move apart.

# Use
1. Compile the solver(15puzzle.c) using the Makefile.
2. Create a text file with initial state of the 15 puzzle, left to right, top to bottom, with the blank tile denoted by 0.
3. Run the solver with the text file as the argument.

# Further Information
Optimisations were implemented based on [Improved Admissable Heuristics](http://www.aaai.org/Papers/AAAI/1996/AAAI96-178.pdf):

1. *Reduce expanded nodes*:
Generated and expanded nodes were reduced by pruning duplicate nodes, by avoiding branching to the move which returns the search back to the parent node (avoiding the inverse move). 

2. *Speed up node generation*:
Instead of calculating the Manhattan distance of each tile iteratively at each call, the heuristic function can be calculated incrementally. As each action/swap will change exactly one tile’s Manhattan distance by ±1, this will require only one calculation. Using this, Table 2 shows a 3.2x increase in speed consistently.

See the report for table of results.
