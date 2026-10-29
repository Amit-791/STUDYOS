#include "Graph.h"
#include <queue>
#include <stack>
#include <algorithm>
using namespace std;

void DependencyGraph::addNode(int taskId) {
    nodes.insert(taskId);
    if (adjList.find(taskId) == adjList.end()) {
        adjList[taskId] = vector<int>();
    }
}

void DependencyGraph::removeNode(int taskId) {
    nodes.erase(taskId);
    adjList.erase(taskId);
    for (auto& p : adjList) {
        auto& v = p.second;
        v.erase(remove(v.begin(), v.end(), taskId), v.end());
    }
}

void DependencyGraph::addEdge(int prerequisite, int dependentTask) {
    addNode(prerequisite);
    addNode(dependentTask);
    adjList[prerequisite].push_back(dependentTask);
}

void DependencyGraph::removeEdge(int prerequisite, int dependentTask) {
    auto it = adjList.find(prerequisite);
    if (it == adjList.end()) return;
    auto& v = it->second;
    v.erase(remove(v.begin(), v.end(), dependentTask), v.end());
}

bool DependencyGraph::hasEdge(int prerequisite, int dependentTask) const {
    auto it = adjList.find(prerequisite);
    if (it == adjList.end()) return false;
    for (int v : it->second) if (v == dependentTask) return true;
    return false;
}

vector<int> DependencyGraph::getDependents(int taskId) const {
    auto it = adjList.find(taskId);
    if (it == adjList.end()) return {};
    return it->second;
}

vector<int> DependencyGraph::getPrerequisites(int taskId) const {
    vector<int> result;
    for (const auto& p : adjList) {
        for (int v : p.second) {
            if (v == taskId) result.push_back(p.first);
        }
    }
    return result;
}

vector<int> DependencyGraph::bfs(int startId) const {
    vector<int> visitedOrder;
    unordered_set<int> visited;
    queue<int> q;
    q.push(startId);
    visited.insert(startId);

    while (!q.empty()) {
        int cur = q.front(); q.pop();
        visitedOrder.push_back(cur);
        auto it = adjList.find(cur);
        if (it != adjList.end()) {
            for (int next : it->second) {
                if (!visited.count(next)) {
                    visited.insert(next);
                    q.push(next);
                }
            }
        }
    }
    return visitedOrder;
}

vector<int> DependencyGraph::dfs(int startId) const {
    vector<int> visitedOrder;
    unordered_set<int> visited;
    stack<int> st;
    st.push(startId);

    while (!st.empty()) {
        int cur = st.top(); st.pop();
        if (visited.count(cur)) continue;
        visited.insert(cur);
        visitedOrder.push_back(cur);
        auto it = adjList.find(cur);
        if (it != adjList.end()) {
            for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
                if (!visited.count(*rit)) st.push(*rit);
            }
        }
    }
    return visitedOrder;
}

bool DependencyGraph::topologicalSort(vector<int>& order) const {
    unordered_map<int, int> indegree;
    for (int n : nodes) indegree[n] = 0;
    for (const auto& p : adjList) {
        for (int v : p.second) {
            indegree[v]++;
        }
    }

    queue<int> q;
    for (int n : nodes) {
        if (indegree[n] == 0) q.push(n);
    }

    order.clear();
    while (!q.empty()) {
        int cur = q.front(); q.pop();
        order.push_back(cur);
        auto it = adjList.find(cur);
        if (it != adjList.end()) {
            for (int next : it->second) {
                indegree[next]--;
                if (indegree[next] == 0) q.push(next);
            }
        }
    }

    return order.size() == nodes.size();   // false => cycle detected
}

vector<pair<int,int>> DependencyGraph::getAllEdges() const {
    vector<pair<int,int>> edges;
    for (const auto& p : adjList) {
        for (int v : p.second) edges.push_back({p.first, v});
    }
    return edges;
}

vector<int> DependencyGraph::impactAnalysis(int taskId) const {
    // Everything reachable from taskId (excluding itself) is impacted by its delay.
    vector<int> reachable = bfs(taskId);
    vector<int> impacted;
    for (int t : reachable) if (t != taskId) impacted.push_back(t);
    return impacted;
}
