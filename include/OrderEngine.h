#ifndef ORDER_ENGINE_H
#define ORDER_ENGINE_H

#include "AppGraph.h"
#include "OrbitCounter.h"

class OrderEngine {
public:
    OrderEngine(const AppGraph& pattern, const OrbitCounts& orbits);
    void run();

    const std::vector<int>& getOrder() const { return order; }
    const std::unordered_map<int, int>& getPivot() const { return pivot; }

private:
    const AppGraph& pattern_graph;
    const OrbitCounts& pattern_orbits;
    std::vector<int> order;
    std::unordered_map<int, int> pivot; // Stores the pivot for each node
};

#endif // ORDER_ENGINE_H

