#ifndef APP_GRAPH_H
#define APP_GRAPH_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "Escape/Graph.h"

// Renamed from Graph to AppGraph to avoid naming collision with Escape::Graph
class AppGraph {
public:
    Escape::CGraph* c_graph = nullptr;
    std::vector<int> original_node_ids;
    std::unordered_map<int, int> node_to_idx;
    std::unordered_map<int, int> labels;

    AppGraph() = default;
    ~AppGraph();
    AppGraph(const AppGraph& other);
    AppGraph& operator=(const AppGraph& other);

    bool readFromFile(const std::string& filepath);
    int degree(int node_id) const;
    bool hasEdge(int u, int v) const;
    std::vector<int> getNeighbors(int node_id) const;
    
    static AppGraph createSubgraph(const AppGraph& original, const std::unordered_set<int>& node_subset);

private:
    void clear();
    void copy_from(const AppGraph& other);
};

#endif // APP_GRAPH_H
