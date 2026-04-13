#include "SearchEngine.h"

// Updated constructor to accept and store the 'induced' flag
SearchEngine::SearchEngine(const AppGraph& data, const AppGraph& pattern, 
                           const CandidateSets& candidates, const std::vector<int>& o,
                           const std::unordered_map<int, int>& p, bool induced)
    : data_graph(data), pattern_graph(pattern), candidate_sets(candidates), order(o), pivot(p), is_induced(induced) {
    
    // --- OPTIMIZATION ---
    // Pre-allocate flat arrays based on the total number of vertices. 
    // This removes all hashing overhead from the inner search loop.
    fast_mapping.assign(pattern_graph.node_to_idx.size(), -1);
    fast_inverse_mapping.assign(data_graph.node_to_idx.size(), -1);
}

void SearchEngine::run() {
    matches.clear();
    std::fill(fast_mapping.begin(), fast_mapping.end(), -1);
    std::fill(fast_inverse_mapping.begin(), fast_inverse_mapping.end(), -1);
    backtrack(0);
}

bool SearchEngine::is_valid(int u, int v) {
    // O(1) array lookup instead of unordered_map.count()
    if (fast_inverse_mapping[v] != -1) return false;

    // --- PIVOT PRUNING ---
    auto pivot_it = pivot.find(u);
    if (pivot_it != pivot.end()) {
        int pivot_u = pivot_it->second;
        if (fast_mapping[pivot_u] != -1) {
            int mapped_pivot_v = fast_mapping[pivot_u];
            if (!data_graph.hasEdge(v, mapped_pivot_v)) {
                return false;
            }
        }
    }

    // --- FULL ADJACENCY CHECK ---
    for (size_t u_prev = 0; u_prev < fast_mapping.size(); ++u_prev) {
        int v_prev = fast_mapping[u_prev];
        if (v_prev == -1) continue; // Skip unmapped vertices
        
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
        // --- OPTIMIZATION ---
        // Convert the fast array back into an unordered_map ONLY when a match is verified.
        Mapping current_mapping;
        for (size_t i = 0; i < fast_mapping.size(); ++i) {
            if (fast_mapping[i] != -1) {
                current_mapping[i] = fast_mapping[i];
            }
        }
        matches.push_back(current_mapping);
        return;
    }

    int u = order[depth];
    for (int v : candidate_sets.at(u)) {
        if (is_valid(u, v)) {
            // Apply fast mapping
            fast_mapping[u] = v;
            fast_inverse_mapping[v] = u;
            
            backtrack(depth + 1);

            // Revert mapping
            fast_mapping[u] = -1;
            fast_inverse_mapping[v] = -1;
        }
    }
}