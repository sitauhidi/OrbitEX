#ifndef FILTER_ENGINE_H
#define FILTER_ENGINE_H

#include "AppGraph.h"
#include "OrbitCounter.h"
#include <string>

using CandidateSets = std::unordered_map<int, std::vector<int>>;

class FilterEngine {
public:
    // Updated constructor to accept the new mode flag
    FilterEngine(const AppGraph& data, const AppGraph& pattern, int g_size, bool use_full_graph_mode);

    bool run();
    const CandidateSets& getCandidateSets() const { return candidate_sets; }
    const OrbitCounts& getPatternOrbits() const { return pattern_orbits; }
    const AppGraph& getCandidateSubgraph() const { return candidate_subgraph; }

    void countCandidates(const std::string& filter_step_name) const;

private:
    bool ldfFilter();
    bool nlfFilter();
    bool orbitFilter();

    const AppGraph& data_graph;
    const AppGraph& pattern_graph;
    int graphlet_size;
    bool use_subgraph; // True if we should use subgraphing, false otherwise
    
    CandidateSets candidate_sets;
    OrbitCounts pattern_orbits;
    AppGraph candidate_subgraph;
};

#endif // FILTER_ENGINE_H

