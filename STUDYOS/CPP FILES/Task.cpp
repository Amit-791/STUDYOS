#include "Task.h"
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;

// ---------------- Enum helpers ----------------
string taskTypeToString(TaskType t) {
    switch (t) {
        case TaskType::ASSIGNMENT: return "ASSIGNMENT";
        case TaskType::QUIZ:       return "QUIZ";
        case TaskType::EXAM:       return "EXAM";
        case TaskType::PROJECT:    return "PROJECT";
        case TaskType::LAB:        return "LAB";
        case TaskType::PERSONAL:   return "PERSONAL";
    }
    return "ASSIGNMENT";
}

TaskType stringToTaskType(const string& s) {
    if (s == "QUIZ") return TaskType::QUIZ;
    if (s == "EXAM") return TaskType::EXAM;
    if (s == "PROJECT") return TaskType::PROJECT;
    if (s == "LAB") return TaskType::LAB;
    if (s == "PERSONAL") return TaskType::PERSONAL;
    return TaskType::ASSIGNMENT;
}

string taskStatusToString(TaskStatus s) {
    switch (s) {
        case TaskStatus::PENDING:     return "PENDING";
        case TaskStatus::IN_PROGRESS: return "IN_PROGRESS";
        case TaskStatus::COMPLETED:   return "COMPLETED";
        case TaskStatus::OVERDUE:     return "OVERDUE";
    }
    return "PENDING";
}

TaskStatus stringToTaskStatus(const string& s) {
    if (s == "IN_PROGRESS") return TaskStatus::IN_PROGRESS;
    if (s == "COMPLETED") return TaskStatus::COMPLETED;
    if (s == "OVERDUE") return TaskStatus::OVERDUE;
    return TaskStatus::PENDING;
}

// ---------------- Task base ----------------
Task::Task(int id, string title, string subject, TaskType type, int deadlineDays, int workloadHours, int priority)
    : id(id), title(move(title)), subject(move(subject)), type(type), deadlineDays(deadlineDays),
      workloadHours(workloadHours), priority(priority), status(TaskStatus::PENDING) {}

double Task::calculateUrgency() const {
    // Closer deadline -> higher urgency. Overdue tasks get max urgency.
    if (deadlineDays <= 0) return 100.0;
    return 100.0 / (deadlineDays + 1);
}

double Task::calculateRisk(int pendingDependencies) const {
    // Risk score combines: urgency, task-type weight, user priority,
    // workload pressure and unresolved dependencies.
    double urgency = calculateUrgency();
    double risk = urgency * getTypeWeight();
    risk += priority * 2.0;
    risk += workloadHours * 0.5;
    risk += pendingDependencies * 3.0;   // blocked tasks are riskier
    if (status == TaskStatus::OVERDUE) risk += 50.0;
    return risk;
}

void Task::display() const {
    cout << left << setw(4) << id
         << setw(12) << getTypeName()
         << setw(22) << title
         << setw(14) << subject
         << "Deadline:" << setw(5) << deadlineDays
         << "Workload:" << setw(4) << workloadHours
         << "Priority:" << setw(2) << priority
         << "Status:" << taskStatusToString(status) << "\n";
}

string Task::serialize() const {
    // id|TYPE|title|subject|deadline|workload|priority|status
    ostringstream oss;
    oss << id << "|" << taskTypeToString(type)
        << "|" << title << "|" << subject << "|" << deadlineDays
        << "|" << workloadHours << "|" << priority << "|" << taskStatusToString(status);
    return oss.str();
}

Task* Task::deserialize(const string& line) {
    // split by '|'
    vector<string> parts;
    stringstream ss(line);
    string item;
    while (getline(ss, item, '|')) parts.push_back(item);
    if (parts.size() < 8) return nullptr;

    int id = stoi(parts[0]);
    TaskType type = stringToTaskType(parts[1]);
    string title = parts[2];
    string subject = parts[3];
    int deadline = stoi(parts[4]);
    int workload = stoi(parts[5]);
    int priority = stoi(parts[6]);
    TaskStatus status = stringToTaskStatus(parts[7]);

    Task* t = createTask(type, id, title, subject, deadline, workload, priority);
    if (t) t->setStatus(status);
    return t;
}

// ---------------- Derived classes ----------------
Assignment::Assignment(int id, string title, string subject, int deadlineDays, int workloadHours, int priority)
    : Task(id, move(title), move(subject), TaskType::ASSIGNMENT, deadlineDays, workloadHours, priority) {}

Quiz::Quiz(int id, string title, string subject, int deadlineDays, int workloadHours, int priority)
    : Task(id, move(title), move(subject), TaskType::QUIZ, deadlineDays, workloadHours, priority) {}

Exam::Exam(int id, string title, string subject, int deadlineDays, int workloadHours, int priority)
    : Task(id, move(title), move(subject), TaskType::EXAM, deadlineDays, workloadHours, priority) {}

Project::Project(int id, string title, string subject, int deadlineDays, int workloadHours, int priority)
    : Task(id, move(title), move(subject), TaskType::PROJECT, deadlineDays, workloadHours, priority) {}

LabWork::LabWork(int id, string title, string subject, int deadlineDays, int workloadHours, int priority)
    : Task(id, move(title), move(subject), TaskType::LAB, deadlineDays, workloadHours, priority) {}

PersonalActivity::PersonalActivity(int id, string title, string subject, int deadlineDays, int workloadHours, int priority)
    : Task(id, move(title), move(subject), TaskType::PERSONAL, deadlineDays, workloadHours, priority) {}

// ---------------- Factory ----------------
Task* createTask(TaskType type, int id, string title, string subject, int deadlineDays, int workloadHours, int priority) {
    switch (type) {
        case TaskType::ASSIGNMENT: return new Assignment(id, title, subject, deadlineDays, workloadHours, priority);
        case TaskType::QUIZ:       return new Quiz(id, title, subject, deadlineDays, workloadHours, priority);
        case TaskType::EXAM:       return new Exam(id, title, subject, deadlineDays, workloadHours, priority);
        case TaskType::PROJECT:    return new Project(id, title, subject, deadlineDays, workloadHours, priority);
        case TaskType::LAB:        return new LabWork(id, title, subject, deadlineDays, workloadHours, priority);
        case TaskType::PERSONAL:   return new PersonalActivity(id, title, subject, deadlineDays, workloadHours, priority);
    }
    return nullptr;
}
