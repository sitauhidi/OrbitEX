#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include "AppGraph.h"
#include "FilterEngine.h"

using Mapping = std::unordered_map<int, int>;

class SearchEngine {
public:
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
    bool is_induced;

    std::vector<int> fast_mapping;
    std::vector<int> fast_inverse_mapping;
    
    std::vector<std::vector<int>> pattern_neighbors;
    std::vector<std::vector<int>> pattern_non_neighbors;

    std::vector<std::vector<uint64_t>> bit_candidates;
    int num_words = 0;
    std::vector<int> order_position;
    std::vector<std::vector<int>> conflict_set;
    bool enable_cbj = false;

    bool enable_symmetry_breaking = false;
    std::vector<int> symmetry_parent;

    int backtrack_cbj(int depth);
    bool is_valid_cbj(int u, int v, int depth, int& conflict_u);

    std::vector<Mapping> matches;
};

#endif