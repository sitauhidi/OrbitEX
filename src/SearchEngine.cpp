#include "SearchEngine.h"

// Updated constructor to accept and store the 'induced' flag
SearchEngine::SearchEngine(const AppGraph& data, const AppGraph& pattern, 
                           const CandidateSets& candidates, const std::vector<int>& o,
                           const std::unordered_map<int, int>& p, bool induced)
    : data_graph(data), pattern_graph(pattern), candidate_sets(candidates), order(o), pivot(p), is_induced(induced) {}

void SearchEngine::run() {
    matches.clear();
    current_mapping.clear();
    inverse_mapping.clear();
    backtrack(0);
}

bool SearchEngine::is_valid(int u, int v) {
    if (inverse_mapping.count(v)) return false;

    // --- PIVOT PRUNING ---
    auto pivot_it = pivot.find(u);
    if (pivot_it != pivot.end()) {
        int pivot_u = pivot_it->second;
        auto mapping_it = current_mapping.find(pivot_u);
        if (mapping_it != current_mapping.end()) {
            int mapped_pivot_v = mapping_it->second;
            if (!data_graph.hasEdge(v, mapped_pivot_v)) {
                return false;
            }
        }
    }

    // --- FULL ADJACENCY CHECK ---
    for (const auto& pair : current_mapping) {
        int u_prev = pair.first;
        int v_prev = pair.second;
        
        if (is_induced) {
            // Induced Subgraph Isomorphism:
            // Edge existence must be identical between the pattern and the data subgraph.
            if (pattern_graph.hasEdge(u, u_prev) != data_graph.hasEdge(v, v_prev)) {
                return false;
            }
        } else {
            // Standard (Non-induced) Subgraph Isomorphism:
            // If an edge exists in the pattern, it must also exist in the data graph mapping.
            // The data graph is allowed to have extra edges.
            if (pattern_graph.hasEdge(u, u_prev)) {
                if (!data_graph.hasEdge(v, v_prev)) {
                    return false;
                }
            }
        }
    }
    return true;
}

void SearchEngine::backtrack(int depth) {
    if (static_cast<size_t>(depth) == order.size()) {
        matches.push_back(current_mapping);
        return;
    }

    int u = order[depth];
    for (int v : candidate_sets.at(u)) {
        if (is_valid(u, v)) {
            current_mapping[u] = v;
            inverse_mapping[v] = u;
            
            backtrack(depth + 1);

            current_mapping.erase(u);
            inverse_mapping.erase(v);
        }
    }
}

