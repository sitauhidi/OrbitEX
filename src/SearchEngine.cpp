#include "SearchEngine.h"

SearchEngine::SearchEngine(const AppGraph& data, const AppGraph& pattern,
                           const CandidateSets& candidates, const std::vector<int>& o,
                           const std::unordered_map<int, int>& p, bool induced, bool cbj)
    : data_graph(data), pattern_graph(pattern), order(o), pivot(p), is_induced(induced), enable_cbj(cbj) {
    
    int max_v_id = 0;
    for (int v_id : data_graph.original_node_ids) {
        if (v_id > max_v_id) max_v_id = v_id;
    }
    fast_mapping.assign(pattern_graph.original_node_ids.size(), -1);
    fast_inverse_mapping.assign(max_v_id + 1, -1);

    size_t num_p_nodes = pattern_graph.original_node_ids.size();
    pattern_neighbors.resize(num_p_nodes);
    pattern_non_neighbors.resize(num_p_nodes);

    for (size_t u = 0; u < num_p_nodes; ++u) {
        int u_id = pattern_graph.original_node_ids[u];
        for (size_t u2 = 0; u2 < num_p_nodes; ++u2) {
            if (u == u2) continue;
            int u2_id = pattern_graph.original_node_ids[u2];
            if (pattern_graph.hasEdge(u_id, u2_id)) {
                pattern_neighbors[u].push_back(u2);
            } else {
                pattern_non_neighbors[u].push_back(u2);
            }
        }
    }

    order_position.assign(num_p_nodes, -1);
    for (size_t d = 0; d < order.size(); ++d) {
        order_position[order[d]] = static_cast<int>(d);
    }

    conflict_set.resize(order.size());

    num_words = (max_v_id + 64) / 64;
    bit_candidates.assign(num_p_nodes, std::vector<uint64_t>(num_words, 0ULL));

    for (const auto& pair : candidates) {
        int u = pair.first;
        if (u >= 0 && static_cast<size_t>(u) < num_p_nodes) {
            for (int v : pair.second) {
                if (v >= 0 && v <= max_v_id) {
                    bit_candidates[u][v / 64] |= (1ULL << (v % 64));
                }
            }
        }
    }
}

void SearchEngine::run() {
    matches.clear();
    std::fill(fast_mapping.begin(), fast_mapping.end(), -1);
    std::fill(fast_inverse_mapping.begin(), fast_inverse_mapping.end(), -1);
    if (enable_cbj) {
        backtrack_cbj(0);
    } else {
        backtrack(0);
    }
}

bool SearchEngine::is_valid(int u, int v) {
    if (fast_inverse_mapping[v] != -1) return false;

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

    for (int u_nb : pattern_neighbors[u]) {
        int v_nb = fast_mapping[u_nb];
        if (v_nb != -1) {
            if (!data_graph.hasEdge(v, v_nb)) {
                return false;
            }
        }
    }

    if (is_induced) {
        for (int u_non_nb : pattern_non_neighbors[u]) {
            int v_non_nb = fast_mapping[u_non_nb];
            if (v_non_nb != -1) {
                if (data_graph.hasEdge(v, v_non_nb)) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool SearchEngine::is_valid_cbj(int u, int v, int depth, int& conflict_u) {
    int current_mapped_u = fast_inverse_mapping[v];
    if (current_mapped_u != -1) {
        conflict_u = current_mapped_u;
        return false;
    }

    auto pivot_it = pivot.find(u);
    if (pivot_it != pivot.end()) {
        int pivot_u = pivot_it->second;
        if (fast_mapping[pivot_u] != -1) {
            int mapped_pivot_v = fast_mapping[pivot_u];
            if (!data_graph.hasEdge(v, mapped_pivot_v)) {
                conflict_u = pivot_u;
                return false;
            }
        }
    }

    for (int u_nb : pattern_neighbors[u]) {
        int v_nb = fast_mapping[u_nb];
        if (v_nb != -1) {
            if (!data_graph.hasEdge(v, v_nb)) {
                conflict_u = u_nb;
                return false;
            }
        }
    }

    if (is_induced) {
        for (int u_non_nb : pattern_non_neighbors[u]) {
            int v_non_nb = fast_mapping[u_non_nb];
            if (v_non_nb != -1) {
                if (data_graph.hasEdge(v, v_non_nb)) {
                    conflict_u = u_non_nb;
                    return false;
                }
            }
        }
    }

    return true;
}

void SearchEngine::backtrack(int depth) {
    if (static_cast<size_t>(depth) == order.size()) {
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
    const auto& words = bit_candidates[u];
    for (int w = 0; w < num_words; ++w) {
        uint64_t bits = words[w];
        while (bits) {
            int v = w * 64 + __builtin_ctzll(bits);
            bits &= bits - 1;

            if (is_valid(u, v)) {
                fast_mapping[u] = v;
                fast_inverse_mapping[v] = u;

                backtrack(depth + 1);

                fast_mapping[u] = -1;
                fast_inverse_mapping[v] = -1;
            }
        }
    }
}

int SearchEngine::backtrack_cbj(int depth) {
    if (static_cast<size_t>(depth) == order.size()) {
        Mapping current_mapping;
        for (size_t i = 0; i < fast_mapping.size(); ++i) {
            if (fast_mapping[i] != -1) {
                current_mapping[i] = fast_mapping[i];
            }
        }
        matches.push_back(current_mapping);
        return depth - 1;
    }

    int u = order[depth];
    conflict_set[depth].clear();
    size_t matches_before = matches.size();

    const auto& words = bit_candidates[u];
    for (int w = 0; w < num_words; ++w) {
        uint64_t bits = words[w];
        while (bits) {
            int v = w * 64 + __builtin_ctzll(bits);
            bits &= bits - 1;

            int conflict_u = -1;
            if (is_valid_cbj(u, v, depth, conflict_u)) {
                fast_mapping[u] = v;
                fast_inverse_mapping[v] = u;

                int backjump_depth = backtrack_cbj(depth + 1);

                fast_mapping[u] = -1;
                fast_inverse_mapping[v] = -1;

                if (backjump_depth >= 0 && backjump_depth < depth) {
                    conflict_set[depth].push_back(order[backjump_depth]);
                }

                if (backjump_depth < depth - 1) {
                    if (backjump_depth >= 0) {
                        conflict_set[backjump_depth].insert(conflict_set[backjump_depth].end(),
                                                             conflict_set[depth].begin(),
                                                             conflict_set[depth].end());
                    }
                    return backjump_depth;
                }
            } else {
                if (conflict_u != -1) {
                    conflict_set[depth].push_back(conflict_u);
                }
            }
        }
    }

    int max_conflict_depth = -1;
    for (int conf_u : conflict_set[depth]) {
        int d_conf = order_position[conf_u];
        if (d_conf > max_conflict_depth && d_conf < depth) {
            max_conflict_depth = d_conf;
        }
    }

    int target_depth = (max_conflict_depth != -1) ? max_conflict_depth : (depth - 1);
    if (matches.size() > matches_before) {
        target_depth = depth - 1;
    }
    if (target_depth >= 0 && target_depth < static_cast<int>(depth)) {
        conflict_set[target_depth].insert(conflict_set[target_depth].end(),
                                           conflict_set[depth].begin(),
                                           conflict_set[depth].end());
    }

    return target_depth;
}