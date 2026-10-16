#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "Task.h"
#include "Graph.h"
#include <unordered_map>
#include <vector>
#include <string>
using namespace std;

class TaskManager {
private:
    unordered_map<int, Task*> taskTable;   
    DependencyGraph depGraph;               
    int nextId;

public:
    TaskManager();
    ~TaskManager();

    // ---- Core CRUD ----
    int addTask(TaskType type, const string& title, const string& subject,
                int deadlineDays, int workloadHours, int priority);
    void insertLoadedTask(Task* t);         // FileStorage on load
    bool removeTask(int id);
    Task* getTask(int id) const;              // hash table lookup
    vector<Task*> getAllTasks() const;
    bool markCompleted(int id);

    // ---- Dependency management ----
    bool addDependency(int prerequisiteId, int dependentId);   
    void addDependencyRaw(int prerequisiteId, int dependentId); // used when loading from file (no cycle re-check needed)
    int countPendingDependencies(int taskId) const;             // incomplete prerequisites
    vector<pair<int,int>> getAllDependencyEdges() const;

    // ---- Graph based analysis ----
    bool getValidExecutionOrder(vector<int>& order) const;      // topological sort
    vector<int> getImpactedTasks(int taskId) const;              // BFS impact analysis

    // ---- Sorting algorithms ----
    vector<Task*> sortByDeadline() const;
    vector<Task*> sortByPriority() const;
    vector<Task*> sortByWorkload() const;
    vector<Task*> sortByRisk() const;

    // ---- Priority queue / heap ----
    vector<Task*> getTopUrgentTasks(int k) const;   // top-k most urgent/risky tasks

    // ---- Greedy scheduling ----
    vector<Task*> generateDailyPlan(int availableHours) const;

    // ---- Risk analysis engine ----
    void printRiskReport() const;

    // ---- What-if Delay Simulator ----
    void simulateDelay(int taskId, int delayDays) const;

    // ---- Dynamic regeneration ----
    void regeneratePlan(int availableHours) const;   // called after completion / new task

    int getNextId() const { return nextId; }
    void setNextId(int n) { nextId = n; }
};

#endif
