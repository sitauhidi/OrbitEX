# OrbitEX: An EXtension to the OrbitSI Subgraph Isomorphism algorithm

OrbitEX is a C++ implementation of the OrbitSI subgraph isomorphism algorithm. It finds all occurrences of a small pattern graph within a larger data graph.

## Requirements
To compile and run this project, you will need:

- A C++ compiler supporting the C++17 standard (e.g., `g++ 8` or newer).
- The `make` build automation tool.
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
- `--graphlet-size <3|4|5>`: (Optional) The size of the graphlets to use for orbit counting (3, 4, or 5). Defaults to 4.
- `--iterate <N>`: (Optional) Set maximum filtering iterations (NLF + Orbit Filter). `N=1` runs 1 pass (default). `N=0` iterates until candidate sets reach convergence (fixpoint). Cannot be combined with `--use-full-graph`.
- `--induced`: (Optional) If present, performs an induced subgraph isomorphism search. By default, it performs a standard non-induced subgraph isomorphism search.
- `--use-full-graph`: (Optional) Perform orbit counting and search backtracking on the full data graph instead of candidate subgraphs. Cannot be combined with `--iterate`.
- `--verbose`: (Optional) Print all matching node mappings to the console.

#### Examples

##### Run a standard subgraph search using 4-node graphlets (Candidate Subgraph Orbit Filtering)
```bash
./build/orbitsi --data test/data_graph/HPRD.graph --pattern test/query_graph/query_dense_16_104.graph
```

##### Run a search using 5-node graphlets with full graph orbit filtering and verbose output
```bash
./build/orbitsi --data test/data_graph/HPRD.graph --pattern test/query_graph/query_dense_16_104.graph --graphlet-size 5 --use-full-graph --verbose
```

##### Run a search with iterative candidate set filtering until convergence (N=0)
```bash
./build/orbitsi --data test/data_graph/HPRD.graph --pattern test/query_graph/query_dense_16_104.graph --iterate 0
```

## Testing
The project includes a parameterized test suite (`test/test_orbitsi.py`) to verify match counts against `test/expected_output.res`.

To run the test suite using default 4-node graphlets:

```bash
make test
```

To run the test suite for specific graphlet sizes (3, 4, or 5):

```bash
GRAPHLET_SIZE=3 pytest test/
GRAPHLET_SIZE=4 pytest test/
GRAPHLET_SIZE=5 pytest test/
```

## Cleaning
To remove all compiled object files and the final executable, run:

```bash
make clean
```
