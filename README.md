# OrbitEX: An EXtension to the OrbitSI Subgraph Isomorphism algorithm

OrbitEX is a C++ implementation of the OrbitSI subgraph isomorphism algorithm. It finds all occurrences of a small pattern graph within a larger data graph.

## Requirements
To compile and run this project, you will need:

- A C++ compiler supporting the C++17 standard (e.g., `g++ 8` or newer).
- The `make` build automation tool.
- `OpenMP` for parallel processing support (included with `g++`).
- `Python 3` and `pytest` for running the test suite.

## Compilation

To build the executable, simply run the make command from the root directory of the project:

```bash
make
```

This will compile all necessary source files from both the application (`src/`) and the internal Escape library (`extern/Escape/`). The final executable will be created at `build/orbitsi`.

## Usage
Once compiled, you can run the program from the command line.

```bash
./build/orbitsi --data <path_to_data_graph> --pattern <path_to_pattern_graph> [options]
```

#### Arguments
- `--data <path>`: (Required) The file path to the large data graph.
- `--pattern <path>`: (Required) The file path to the smaller pattern graph (query).
- `--graphlet-size <3|4|5>`: (Optional) The size of the graphlets to use for orbit counting. Defaults to 4.
- `--induced`: (Optional) If this flag is present, the search will find matches for induced subgraph isomorphism. By default, it performs a standard (non-induced) subgraph isomorphism search
- `--use-full-graph`: (Optional) Use the full data graph for orbit filtering instead of a subgraph
- `--verbose`: (Optional) Print all found matches to the console

##### Run a standard subgraph search using 4-node graphlets

```bash
./build/orbitsi --data test/data_graph/data.graph --pattern test/query_graph/query_triangle.graph
```

##### Run an induced subgraph search using 5-node graphlets on full graph and explicit output

```bash
./build/orbitsi --data test/data_graph/data.graph --pattern test/query_graph/query_square.graph --graphlet-size 5 --induced --use-full-graph --verbose
```

## Testing
The project includes a test suite to verify the correctness of the matching algorithm. The tests run the compiled binary against a set of query graphs and compare the number of matches found against a file of expected results.

To run the test suite, use the following command:

```bash
make test
```

## Cleaning
To remove all compiled object files and the final executable, run the clean command:

```bash
make clean
```
