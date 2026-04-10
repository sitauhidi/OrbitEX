#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include "AppGraph.h"
#include "FilterEngine.h" // For CandidateSets typedef

using Mapping = std::unordered_map<int, int>;

class SearchEngine {
public:
    // Added a boolean flag 'induced' to the constructor
    SearchEngine(const AppGraph& data, const AppGraph& pattern, 
                 const CandidateSets& candidates, const std::vector<int>& order,
                 const std::unordered_map<int, int>& pivot, bool induced);

    void run();
    const std::vector<Mapping>& getMatches() const { return matches; }

private:
    void backtrack(int depth);
    bool is_valid(int u, int v);

    const AppGraph& data_graph;
    const AppGraph& pattern_graph;
    const CandidateSets& candidate_sets;
    const std::vector<int>& order;
    const std::unordered_map<int, int>& pivot;
    bool is_induced; // Member to store the search mode

    // --- OPTIMIZATION: Flat arrays replace unordered_map for O(1) lookups ---
    std::vector<int> fast_mapping;
    std::vector<int> fast_inverse_mapping;
    
    std::vector<Mapping> matches;
};

#endif // SEARCH_ENGINE_H