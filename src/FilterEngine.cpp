#include "FilterEngine.h"
#include <numeric> // For std::accumulate
#include <cmath>   // For std::sqrt
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <chrono>

FilterEngine::FilterEngine(const AppGraph& data, const AppGraph& pattern, int g_size)
    : data_graph(data), pattern_graph(pattern), graphlet_size(g_size) {}

bool FilterEngine::run() {
    auto start = std::chrono::high_resolution_clock::now();

    if (!ldfFilter()) {
        countCandidates("After LDF Filter (Failed)");
        return false;
    }
    auto ldf_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ldf_duration = ldf_end - start;
    std::cout << "\nLDF filtering took " << ldf_duration.count() << " ms." << std::endl;
    countCandidates("After LDF Filter");

    auto nlf_start = std::chrono::high_resolution_clock::now();
    if (!nlfFilter()) {
        countCandidates("After NLF Filter (Failed)");
        return false;
    }
    auto nlf_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> nlf_duration = nlf_end - nlf_start;
    std::cout << "\nNLF filtering took " << nlf_duration.count() << " ms." << std::endl;
    countCandidates("After NLF Filter");
    
    auto orbit_start = std::chrono::high_resolution_clock::now();
    if (!orbitFilter()) {
        countCandidates("After Orbit Filter (Failed)");
        return false;
    }
    auto orbit_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> orbit_duration = orbit_end - orbit_start;
    std::cout << "\nOrbit filtering took " << orbit_duration.count() << " ms." << std::endl;
    countCandidates("After Orbit Filter");

    return true;
}

bool FilterEngine::ldfFilter() {
    std::unordered_map<int, std::vector<int>> label_index;
    for (int node : data_graph.original_node_ids) {
        label_index[data_graph.labels.at(node)].push_back(node);
    }

    for (int u : pattern_graph.original_node_ids) {
        int label_u = pattern_graph.labels.at(u);
        int deg_u = pattern_graph.degree(u);

        if (label_index.find(label_u) == label_index.end()) return false;

        std::vector<int> candidates;
        for (int v : label_index.at(label_u)) {
            if (data_graph.degree(v) >= deg_u) {
                candidates.push_back(v);
            }
        }

        if (candidates.empty()) return false;
        candidate_sets[u] = candidates;
    }
    return true;
}

bool FilterEngine::nlfFilter() {
    std::unordered_map<int, std::unordered_map<int, int>> pattern_nlf;
    for (int u : pattern_graph.original_node_ids) {
        for (int nbr : pattern_graph.getNeighbors(u)) {
            pattern_nlf[u][pattern_graph.labels.at(nbr)]++;
        }
    }

    CandidateSets refined_sets;
    for (const auto& pair : candidate_sets) {
        int u = pair.first;
        const auto& candidates_v = pair.second;
        const auto& u_nlf = pattern_nlf.at(u);
        
        std::vector<int> filtered_v;
        for (int v : candidates_v) {
            std::unordered_map<int, int> v_nlf;
            for (int nbr : data_graph.getNeighbors(v)) {
                v_nlf[data_graph.labels.at(nbr)]++;
            }
            
            bool is_valid = true;
            for (const auto& nlf_pair : u_nlf) {
                if (v_nlf[nlf_pair.first] < nlf_pair.second) {
                    is_valid = false;
                    break;
                }
            }
            if (is_valid) {
                filtered_v.push_back(v);
            }
        }

        if (filtered_v.empty()) return false;
        refined_sets[u] = filtered_v;
    }
    candidate_sets = refined_sets;
    return true;
}

bool FilterEngine::orbitFilter() {
    EVOKEOrbitCounter pattern_counter(pattern_graph, graphlet_size);
    pattern_orbits = pattern_counter.count();

    std::unordered_set<int> candidate_nodes;
    for (const auto& pair : candidate_sets) {
        candidate_nodes.insert(pair.second.begin(), pair.second.end());
    }

    candidate_subgraph = AppGraph::createSubgraph(data_graph, candidate_nodes);

    EVOKEOrbitCounter data_counter(candidate_subgraph, graphlet_size);
    OrbitCounts data_orbits = data_counter.count();

    CandidateSets refined_sets;
    for (const auto& pair : candidate_sets) {
        int u = pair.first;
        const auto& orbit_u = pattern_orbits.at(u);
        
        std::vector<int> filtered_v;
        for (int v : pair.second) {
            if (data_orbits.find(v) == data_orbits.end()) continue;
            const auto& orbit_v = data_orbits.at(v);
            
            bool compatible = true;
            for (size_t i = 0; i < orbit_u.size(); ++i) {
                if (orbit_v[i] < orbit_u[i]) {
                    compatible = false;
                    break;
                }
            }
            if (compatible) {
                filtered_v.push_back(v);
            }
        }
        if (filtered_v.empty()) return false;
        refined_sets[u] = filtered_v;
    }
    candidate_sets = refined_sets;
    return true;
}

void FilterEngine::countCandidates(const std::string& filter_step_name) const {
    std::cout << "\n--- Candidate Set Statistics (" << filter_step_name << ") ---" << std::endl;
    if (candidate_sets.empty()) {
        std::cout << "Average candidates per vertex: 0.0" << std::endl;
        std::cout << "Std. dev. of candidates per vertex: 0.0" << std::endl;
        std::cout << "Min candidates for a vertex: 0" << std::endl;
        std::cout << "Max candidates for a vertex: 0" << std::endl;
        std::cout << "-------------------------------------------------" << std::endl;
        return;
    }

    std::vector<double> sizes;
    sizes.reserve(candidate_sets.size());
    for (const auto& pair : candidate_sets) {
        sizes.push_back(static_cast<double>(pair.second.size()));
    }

    // Calculate mean (average)
    double sum = std::accumulate(sizes.begin(), sizes.end(), 0.0);
    double mean = sum / sizes.size();

    // Calculate standard deviation
    double squared_diff_sum = 0.0;
    for (const double size : sizes) {
        squared_diff_sum += (size - mean) * (size - mean);
    }
    double std_dev = std::sqrt(squared_diff_sum / sizes.size());

    // Find min and max
    auto min_max_it = std::minmax_element(sizes.begin(), sizes.end());
    double min_size = *min_max_it.first;
    double max_size = *min_max_it.second;

    std::cout << "Average candidates per vertex: " << mean << std::endl;
    std::cout << "Std. dev. of candidates per vertex: " << std_dev << std::endl;
    std::cout << "Min candidates for a vertex: " << min_size << std::endl;
    std::cout << "Max candidates for a vertex: " << max_size << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
}