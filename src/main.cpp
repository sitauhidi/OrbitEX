#include <iostream>
#include <vector>
#include <string>
#include <chrono>

#include "AppGraph.h"
#include "FilterEngine.h"
#include "OrderEngine.h"
#include "SearchEngine.h"

void print_usage() {
    // Added the optional --induced flag to the usage string
    std::cerr << "Usage: orbitsi --data <path> --pattern <path> [--graphlet-size 4|5] [--induced]\n";
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty()) {
        print_usage();
        return 1;
    }

    std::string data_path, pattern_path;
    int graphlet_size = 4;
    bool is_induced = false; // Flag for search type, defaults to non-induced

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
                return 1;
            }
        // New logic to parse the --induced flag
        } else if (args[i] == "--induced") {
            is_induced = true;
        }
    }

    if (data_path.empty() || pattern_path.empty()) {
        std::cerr << "Error: --data and --pattern paths are required.\n";
        print_usage();
        return 1;
    }
    if (graphlet_size != 4 && graphlet_size != 5) {
        std::cerr << "Error: --graphlet-size must be 4 or 5.\n";
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
    if (is_induced) {
        std::cout << "Search mode: Induced Subgraph Isomorphism" << std::endl;
    }

    auto start = std::chrono::high_resolution_clock::now();

    FilterEngine filter_engine(data_graph, pattern_graph, graphlet_size);
    if (!filter_engine.run()) {
        std::cout << "✅ Matches found: 0" << std::endl;
        return 0;
    }
    std::cout << "\nFiltering complete." << std::endl;

    OrderEngine order_engine(pattern_graph, filter_engine.getPatternOrbits());
    order_engine.run();
    std::cout << "Ordering complete." << std::endl;

    // Pass the is_induced flag to the SearchEngine
    SearchEngine search_engine(filter_engine.getCandidateSubgraph(), pattern_graph,
                               filter_engine.getCandidateSets(), order_engine.getOrder(),
                               order_engine.getPivot(), is_induced);
    search_engine.run();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    const auto& matches = search_engine.getMatches();
    std::cout << "\n✅ Matches found: " << matches.size() << std::endl;
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

    std::cout << "\nTotal execution time: " << elapsed.count() << " seconds." << std::endl;

    return 0;
}

