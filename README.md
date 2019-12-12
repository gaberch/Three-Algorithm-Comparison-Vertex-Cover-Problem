# Vertex Cover Problem Solver Using MiniSAT & Two Approximation Algorithms

Simple program built in C++ that incorporates a MiniSat SAT solver cloned from https://github.com/agurfinkel/minisat and two approximation algorithms working in tandem to solve the minimum vertex cover for any undirected graph. The MiniSAT solver is implemented as described at this link (http://minisat.se/downloads/MiniSat.pdf). The implemented program does the following:
* Takes an undirected graph as an input
* Calculates the minimum vertex cover of the inputted graph in three separate threads (1 for the miniSAT solver and 2 for the two different approximation algorithms)
* Output of MiniSAT solver is denoted as CNF-SAT-VC
* Output of approximation algorithm 1 is denoted as APPROX-VC-1
* Output of approximation algorithm 2 is denoted as APPROX-VC-2

## Approximation Algorithms used

### Approximation Algorithm 1

Pick a vertex of highest degree (most incident edges). Add it to the vertex cover and throw away all edges incident on that vertex. Repeat until no edges remain.

### Approximation Algorithm 2

Pick an edge <u, v>, and add both u and v to your vertex cover. Throw away all edges
attached to u and v. Repeat until no edges remain.

## CMake

The CMake instructions are contained in `CMakeLists.txt`. To compile the program, do the following, starting in the main repo:
```
mkdir build
cd build
cmake ../
make
```

## Sample Run

### Input
```
V 5
E {<2,1>,<2,0>,<2,3>,<1,4>,<4,3>}
```

### Output
```
CNF-SAT-VC: 2,4
APPROX-VC-1: 2,4
APPROX-VC-2: 0,2,3,4
```

In the above sample run, the lines "V = ...", "E = ...." are inputs and "3 4" is the output.


## Encoding to SAT of Vertex Cover Problem

For information about the encoding used as part of this program, go to `encoding.pdf` file in the main repository of this project

## Comparison Report

For a small report comparing the three algorithms, refer to file called `report.pdf` in the main repository of this project.
