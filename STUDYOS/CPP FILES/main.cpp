#include <iostream>
#include <limits>
#include <vector>
#include "Task.h"
#include "TaskManager.h"
#include "FileStorage.h"
using namespace std;

const string TASKS_FILE = "tasks_data.txt";
const string DEPS_FILE = "dependencies_data.txt";

void printBanner() {
    cout << "\n============================================================\n";
    cout << "   STUDYOS - Intelligent Academic Planning System\n";
    cout << "============================================================\n";
}

void printMenu() {
    cout << "\n---------------------------- MENU ----------------------------\n";
    cout << " 1. Add Task\n";
    cout << " 2. Add Dependency (prerequisite -> dependent)\n";
    cout << " 3. View All Tasks\n";
    cout << " 4. View Valid Execution Order (Topological Sort)\n";
    cout << " 5. View Top Urgent Tasks (Priority Queue)\n";
    cout << " 6. Sort Tasks (Deadline / Priority / Workload)\n";
    cout << " 7. Generate Today's Study Plan (Greedy Scheduling)\n";
    cout << " 8. What-if Delay Simulator\n";
    cout << " 9. Mark Task as Completed\n";
    cout << "10. Risk Analysis Report\n";
    cout << "11. Save & Exit\n";
    cout << "----------------------------------------------------------------\n";
    cout << "Enter choice: ";
}

int readInt(const string& prompt) {
    int val;
    cout << prompt;
    while (!(cin >> val)) {
        if (cin.eof()) {
            cout << "\nInput stream ended. Exiting.\n";
            exit(0);
        }
        cout << "Invalid input. Enter a number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');   // consume trailing newline once
    return val;
}

string readLine(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);   // buffer is already clean (readInt consumes its own newline)
    return s;
}

TaskType readTaskType() {
    cout << "Select type: 1.Assignment 2.Quiz 3.Exam 4.Project 5.Lab 6.Personal\n";
    int c = readInt("Choice: ");
    switch (c) {
        case 1: return TaskType::ASSIGNMENT;
        case 2: return TaskType::QUIZ;
        case 3: return TaskType::EXAM;
        case 4: return TaskType::PROJECT;
        case 5: return TaskType::LAB;
        case 6: return TaskType::PERSONAL;
        default: return TaskType::ASSIGNMENT;
    }
}

void displayTaskList(const vector<Task*>& tasks) {
    if (tasks.empty()) {
        cout << "  (no tasks)\n";
        return;
    }
    for (Task* t : tasks) t->display();
}

int main() {
    printBanner();

    TaskManager manager;

    // Load existing data if present
    if (FileStorage::loadTasks(TASKS_FILE, manager)) {
        FileStorage::loadDependencies(DEPS_FILE, manager);
        cout << "Existing data loaded from disk (" << manager.getAllTasks().size() << " tasks).\n";
    } else {
        cout << "No previous data found. Starting fresh.\n";
    }

    bool running = true;
    while (running) {
        printMenu();
        int choice = readInt("");

        switch (choice) {
            case 1: {
                TaskType type = readTaskType();
                string title = readLine("Title: ");
                string subject = readLine("Subject: ");
                int deadline = readInt("Deadline (days from now): ");
                int workload = readInt("Estimated workload (hours): ");
                int priority = readInt("Priority (1-5): ");
                int id = manager.addTask(type, title, subject, deadline, workload, priority);
                cout << "Task added with ID #" << id << "\n";
                manager.regeneratePlan(readInt("Regenerate plan? Enter today's available hours (0 to skip): "));
                break;
            }
            case 2: {
                int a = readInt("Prerequisite Task ID (must be done first): ");
                int b = readInt("Dependent Task ID (depends on above): ");
                if (manager.addDependency(a, b))
                    cout << "Dependency added: Task #" << a << " -> Task #" << b << "\n";
                else
                    cout << "Failed to add dependency (invalid IDs or cycle detected).\n";
                break;
            }
            case 3: {
                cout << "\nAll Tasks:\n";
                displayTaskList(manager.getAllTasks());
                break;
            }
            case 4: {
                vector<int> order;
                if (manager.getValidExecutionOrder(order)) {
                    cout << "\nValid Task Execution Order (Topological Sort):\n";
                    for (int id : order) {
                        Task* t = manager.getTask(id);
                        if (t) cout << "  " << id << ". [" << t->getTypeName() << "] " << t->getTitle() << "\n";
                    }
                } else {
                    cout << "Cycle detected in dependencies! No valid order exists.\n";
                }
                break;
            }
            case 5: {
                int k = readInt("How many top urgent tasks to show? ");
                cout << "\nTop Urgent / High-Risk Tasks:\n";
                displayTaskList(manager.getTopUrgentTasks(k));
                break;
            }
            case 6: {
                cout << "Sort by: 1.Deadline 2.Priority 3.Workload 4.Risk\n";
                int c = readInt("Choice: ");
                vector<Task*> sorted;
                if (c == 1) sorted = manager.sortByDeadline();
                else if (c == 2) sorted = manager.sortByPriority();
                else if (c == 3) sorted = manager.sortByWorkload();
                else sorted = manager.sortByRisk();
                displayTaskList(sorted);
                break;
            }
            case 7: {
                int hours = readInt("Available study hours today: ");
                vector<Task*> plan = manager.generateDailyPlan(hours);
                cout << "\nRecommended plan for today (Greedy Scheduling):\n";
                int used = 0;
                for (Task* t : plan) {
                    cout << "  - [" << t->getTypeName() << "] " << t->getTitle()
                         << " (" << t->getWorkload() << "h, priority " << t->getPriority() << ")\n";
                    used += t->getWorkload();
                }
                if (plan.empty()) cout << "  (nothing schedulable right now)\n";
                cout << "Total hours planned: " << used << " / " << hours << "\n";
                break;
            }
            case 8: {
                int id = readInt("Task ID to delay: ");
                int days = readInt("Delay by how many days? ");
                manager.simulateDelay(id, days);
                break;
            }
            case 9: {
                int id = readInt("Task ID completed: ");
                if (manager.markCompleted(id)) {
                    cout << "Task #" << id << " marked completed.\n";
                    int hrs = readInt("Regenerate today's plan? Enter available hours (0 to skip): ");
                    if (hrs > 0) manager.regeneratePlan(hrs);
                } else {
                    cout << "Task not found.\n";
                }
                break;
            }
            case 10: {
                manager.printRiskReport();
                break;
            }
            case 11: {
                FileStorage::saveTasks(TASKS_FILE, manager);
                FileStorage::saveDependencies(DEPS_FILE, manager);
                cout << "Data saved. Goodbye!\n";
                running = false;
                break;
            }
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}
