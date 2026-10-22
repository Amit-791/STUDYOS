#include "TaskManager.h"
#include "Utils.h"
#include <iostream>
#include <queue>
#include <algorithm>
#include <iomanip>
using namespace std;

TaskManager::TaskManager() : nextId(1) {}

TaskManager::~TaskManager() {
    for (auto& p : taskTable) delete p.second;
}

int TaskManager::addTask(TaskType type, const string& title, const string& subject,
                          int deadlineDays, int workloadHours, int priority) {
    int id = nextId++;
    Task* t = createTask(type, id, title, subject, deadlineDays, workloadHours, priority);
    taskTable[id] = t;
    depGraph.addNode(id);
    return id;
}

void TaskManager::insertLoadedTask(Task* t) {
    if (!t) return;
    taskTable[t->getId()] = t;
    depGraph.addNode(t->getId());
    if (t->getId() >= nextId) nextId = t->getId() + 1;
}

bool TaskManager::removeTask(int id) {
    auto it = taskTable.find(id);
    if (it == taskTable.end()) return false;
    delete it->second;
    taskTable.erase(it);
    depGraph.removeNode(id);
    return true;
}

Task* TaskManager::getTask(int id) const {
    auto it = taskTable.find(id);   // Hash Table lookup -> O(1) average
    if (it == taskTable.end()) return nullptr;
    return it->second;
}

vector<Task*> TaskManager::getAllTasks() const {
    vector<Task*> result;
    result.reserve(taskTable.size());
    for (auto& p : taskTable) result.push_back(p.second);
    return result;
}

bool TaskManager::markCompleted(int id) {
    Task* t = getTask(id);
    if (!t) return false;
    t->setStatus(TaskStatus::COMPLETED);
    return true;
}

bool TaskManager::addDependency(int prerequisiteId, int dependentId) {
    if (!getTask(prerequisiteId) || !getTask(dependentId)) return false;
    depGraph.addEdge(prerequisiteId, dependentId);

    // Verify we haven't created a cycle
    vector<int> order;
    if (!depGraph.topologicalSort(order)) {
        depGraph.removeEdge(prerequisiteId, dependentId);   // undo - keep graph valid
        cout << "  [!] This dependency creates a CYCLE. Reverted.\n";
        return false;
    }
    return true;
}

void TaskManager::addDependencyRaw(int prerequisiteId, int dependentId) {
    depGraph.addEdge(prerequisiteId, dependentId);
}

vector<pair<int,int>> TaskManager::getAllDependencyEdges() const {
    return depGraph.getAllEdges();
}

int TaskManager::countPendingDependencies(int taskId) const {
    vector<int> prereqs = depGraph.getPrerequisites(taskId);
    int pending = 0;
    for (int pid : prereqs) {
        Task* p = getTask(pid);
        if (p && p->getStatus() != TaskStatus::COMPLETED) pending++;
    }
    return pending;
}

bool TaskManager::getValidExecutionOrder(vector<int>& order) const {
    return depGraph.topologicalSort(order);
}

vector<int> TaskManager::getImpactedTasks(int taskId) const {
    return depGraph.impactAnalysis(taskId);
}

// ---------------- Sorting (uses our own Merge Sort - Utils.h) ----------------
vector<Task*> TaskManager::sortByDeadline() const {
    vector<Task*> tasks = getAllTasks();
    mergeSort(tasks, [](Task* a, Task* b) { return a->getDeadline() < b->getDeadline(); });
    return tasks;
}

vector<Task*> TaskManager::sortByPriority() const {
    vector<Task*> tasks = getAllTasks();
    mergeSort(tasks, [](Task* a, Task* b) { return a->getPriority() > b->getPriority(); });
    return tasks;
}

vector<Task*> TaskManager::sortByWorkload() const {
    vector<Task*> tasks = getAllTasks();
    mergeSort(tasks, [](Task* a, Task* b) { return a->getWorkload() < b->getWorkload(); });
    return tasks;
}

vector<Task*> TaskManager::sortByRisk() const {
    vector<Task*> tasks = getAllTasks();
    // capture 'this' to compute pending dependencies for risk
    mergeSort(tasks, [this](Task* a, Task* b) {
        double ra = a->calculateRisk(countPendingDependencies(a->getId()));
        double rb = b->calculateRisk(countPendingDependencies(b->getId()));
        return ra > rb;
    });
    return tasks;
}

// ---------------- Priority Queue / Heap ----------------
vector<Task*> TaskManager::getTopUrgentTasks(int k) const {
    struct Cmp {
        const TaskManager* mgr;
        bool operator()(Task* a, Task* b) const {
            double ra = a->calculateRisk(mgr->countPendingDependencies(a->getId()));
            double rb = b->calculateRisk(mgr->countPendingDependencies(b->getId()));
            return ra < rb;   // max-heap: highest risk on top
        }
    };

    priority_queue<Task*, vector<Task*>, Cmp> pq{Cmp{this}};
    for (auto& p : taskTable) {
        if (p.second->getStatus() != TaskStatus::COMPLETED) pq.push(p.second);
    }

    vector<Task*> result;
    while (!pq.empty() && (int)result.size() < k) {
        result.push_back(pq.top());
        pq.pop();
    }
    return result;
}

// ---------------- Greedy Scheduling ----------------
vector<Task*> TaskManager::generateDailyPlan(int availableHours) const {
    // Only tasks whose prerequisites are all completed can be scheduled today.
    vector<Task*> candidates;
    for (auto& p : taskTable) {
        Task* t = p.second;
        if (t->getStatus() == TaskStatus::COMPLETED) continue;
        if (countPendingDependencies(t->getId()) > 0) continue;   // blocked
        candidates.push_back(t);
    }

    // Greedy: rank by risk (urgency+priority+workload+deps), pick highest risk
    // per hour of work first, fit as many as possible into availableHours.
    mergeSort(candidates, [this](Task* a, Task* b) {
        double ra = a->calculateRisk(countPendingDependencies(a->getId())) / max(1, a->getWorkload());
        double rb = b->calculateRisk(countPendingDependencies(b->getId())) / max(1, b->getWorkload());
        return ra > rb;
    });

    vector<Task*> plan;
    int hoursUsed = 0;
    for (Task* t : candidates) {
        if (hoursUsed + t->getWorkload() <= availableHours) {
            plan.push_back(t);
            hoursUsed += t->getWorkload();
        }
    }
    return plan;
}

// ---------------- Risk Analysis Engine ----------------
void TaskManager::printRiskReport() const {
    vector<Task*> ranked = sortByRisk();
    cout << "\n===================== RISK ANALYSIS REPORT =====================\n";
    cout << left << setw(4) << "ID" << setw(12) << "Type" << setw(22) << "Title"
         << setw(10) << "Deadline" << setw(10) << "PendDeps" << "Risk Score\n";
    cout << "------------------------------------------------------------------\n";
    for (Task* t : ranked) {
        if (t->getStatus() == TaskStatus::COMPLETED) continue;
        int pending = countPendingDependencies(t->getId());
        double risk = t->calculateRisk(pending);
        cout << left << setw(4) << t->getId() << setw(12) << t->getTypeName()
             << setw(22) << t->getTitle() << setw(10) << t->getDeadline()
             << setw(10) << pending << fixed << setprecision(2) << risk << "\n";
    }
    cout << "===================================================================\n";
}

// ---------------- What-if Delay Simulator ----------------
void TaskManager::simulateDelay(int taskId, int delayDays) const {
    Task* t = getTask(taskId);
    if (!t) {
        cout << "Task not found.\n";
        return;
    }

    vector<int> impacted = getImpactedTasks(taskId);

    cout << "\n================ WHAT-IF DELAY SIMULATION ================\n";
    cout << "Delaying Task #" << taskId << " (" << t->getTitle() << ") by "
         << delayDays << " day(s).\n";
    cout << "New effective deadline pressure: " << (t->getDeadline() - delayDays) << " days left\n\n";

    if (impacted.empty()) {
        cout << "No dependent tasks are affected.\n";
    } else {
        cout << "Tasks impacted (via dependency graph BFS):\n";
        cout << left << setw(6) << "ID" << setw(22) << "Title" << setw(10)
             << "Deadline" << "New Slack (days)\n";
        for (int id : impacted) {
            Task* dep = getTask(id);
            if (!dep) continue;
            int newSlack = dep->getDeadline() - delayDays;
            cout << left << setw(6) << dep->getId() << setw(22) << dep->getTitle()
                 << setw(10) << dep->getDeadline() << newSlack;
            if (newSlack < 0) cout << "   [!] DEADLINE WILL BE MISSED";
            else if (newSlack <= 1) cout << "   [!] HIGH RISK - very tight";
            cout << "\n";
        }
    }
    cout << "============================================================\n";
}

// ---------------- Dynamic regeneration ----------------
void TaskManager::regeneratePlan(int availableHours) const {
    cout << "\n[Regenerating daily plan after task update...]\n";
    vector<Task*> plan = generateDailyPlan(availableHours);
    if (plan.empty()) {
        cout << "No schedulable tasks right now (either all done or blocked by dependencies).\n";
        return;
    }
    int hoursUsed = 0;
    cout << "Updated recommended plan:\n";
    for (Task* t : plan) {
        cout << "  - [" << t->getTypeName() << "] " << t->getTitle()
             << " (" << t->getWorkload() << "h)\n";
        hoursUsed += t->getWorkload();
    }
    cout << "Total hours planned: " << hoursUsed << " / " << availableHours << "\n";
}
