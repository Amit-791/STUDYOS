#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <vector>
#include <unordered_set>
using namespace std;

// Directed dependency graph.
// Edge (u -> v) means: task u must be completed BEFORE task v can start
// (u is a prerequisite of v).
class DependencyGraph {
private:
    unordered_map<int, vector<int>> adjList;   // u -> list of v (v depends on u)
    unordered_set<int> nodes;                    // all task ids currently in graph

public:
    void addNode(int taskId);
    void removeNode(int taskId);
    void addEdge(int prerequisite, int dependentTask);
    void removeEdge(int prerequisite, int dependentTask);
    bool hasEdge(int prerequisite, int dependentTask) const;

    vector<int> getPrerequisites(int taskId) const;   // tasks that must finish before taskId
    vector<int> getDependents(int taskId) const;       // tasks that depend on taskId (adjList[taskId])

    vector<int> bfs(int startId) const;   // all tasks reachable (impacted) forward from startId
    vector<int> dfs(int startId) const;   // same, depth-first

    // Kahn's algorithm. Returns true if a valid order exists (no cycle),
    // fills 'order' with the valid task execution sequence.
    bool topologicalSort(vector<int>& order) const;

    // BFS-based impact analysis: if 'taskId' is delayed, which tasks are affected?
    vector<int> impactAnalysis(int taskId) const;

    const unordered_set<int>& getAllNodes() const { return nodes; }
    vector<pair<int,int>> getAllEdges() const;
};

#endif
