#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "AppGraph.h"
#include "FilterEngine.h"
#include "OrderEngine.h"
#include "SearchEngine.h"

void print_usage() {
    std::cerr << "Usage: ./build/orbitsi --data <path> --pattern <path> [options]\n\n"
              << "Options:\n"
              << "  --graphlet-size <3|4|5>  Set the graphlet size for orbit counting (default: 4)\n"
              << "  --induced                Perform an induced subgraph isomorphism search\n"
              << "  --use-full-graph         Use the full data graph for orbit filtering instead of a subgraph\n"
              << "  --verbose                Print all found matches to the console\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty()) {
        print_usage();
        return 1;
    }

    std::string data_path, pattern_path;
    int graphlet_size = 4;
    bool induced_search = false;
    bool use_full_graph = false;
    bool verbose = false; // New flag for verbose output

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--data" && i + 1 < args.size()) {
            data_path = args[++i];
        } else if (args[i] == "--pattern" && i + 1 < args.size()) {
            pattern_path = args[++i];
        } else if (args[i] == "--graphlet-size" && i + 1 < args.size()) {
            try {
                graphlet_size = std::stoi(args[++i]);
            } catch(...) {
                std::cerr << "Invalid graphlet size.\n";
                print_usage();
                return 1;
            }
        } else if (args[i] == "--induced") {
            induced_search = true;
        } else if (args[i] == "--use-full-graph") {
            use_full_graph = true;
        } else if (args[i] == "--verbose") {
            verbose = true;
        }
    }

    if (data_path.empty() || pattern_path.empty()) {
        std::cerr << "Error: --data and --pattern paths are required.\n";
        print_usage();
        return 1;
    }
    if (graphlet_size != 3 && graphlet_size != 4 && graphlet_size != 5) {
        std::cerr << "Error: --graphlet-size must be 3, 4, or 5.\n";
        print_usage();
        return 1;
    }

    AppGraph data_graph, pattern_graph;
    if (!data_graph.readFromFile(data_path)) {
        std::cerr << "Error reading data graph from " << data_path << std::endl;
        return 1;
    }
    if (!pattern_graph.readFromFile(pattern_path)) {
        std::cerr << "Error reading pattern graph from " << pattern_path << std::endl;
        return 1;
    }
    std::cout << "Graphs loaded successfully." << std::endl;
    std::cout << "Search type: " << (induced_search ? "Induced" : "Non-Induced") << std::endl;
    std::cout << "Graphlet size: " << graphlet_size << std::endl;
    std::cout << "Filter mode: " << (use_full_graph ? "Full Graph" : "Subgraph") << std::endl;


    auto start = std::chrono::high_resolution_clock::now();

    // Pass the new flag to the FilterEngine
    FilterEngine filter_engine(data_graph, pattern_graph, graphlet_size, use_full_graph);
    if (!filter_engine.run()) {
        std::cout << "Matches found: 0" << std::endl;
        return 0;
    }
    
    auto filter_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> filter_elapsed = filter_end - start;
    std::cout << "\nTotal filtering time: " << filter_elapsed.count() << " seconds." << std::endl;


    OrderEngine order_engine(pattern_graph, filter_engine.getPatternOrbits());
    order_engine.run();

    SearchEngine search_engine(filter_engine.getCandidateSubgraph(), pattern_graph,
                               filter_engine.getCandidateSets(), order_engine.getOrder(),
                               order_engine.getPivot(), induced_search);
    search_engine.run();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_elapsed = end - start;
    
    const auto& matches = search_engine.getMatches();
    std::cout << "\nMatches found: " << matches.size() << std::endl;
    
    // If verbose mode is enabled, print all the matches.
    if (verbose) {
        std::cout << "\n--- Found Matches ---" << std::endl;
        for (const auto& match : matches) {
            std::cout << "{";
            bool first = true;
            for (const auto& pair : match) {
                if (!first) std::cout << ", ";
                std::cout << pair.first << ": " << pair.second;
                first = false;
            }
            std::cout << "}" << std::endl;
        }
        std::cout << "---------------------" << std::endl;
    }

    std::cout << "\nTotal execution time: " << total_elapsed.count() << " seconds." << std::endl;

    return 0;
}

