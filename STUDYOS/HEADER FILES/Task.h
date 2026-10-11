#ifndef TASK_H
#define TASK_H

#include <string>
using namespace std;


enum class TaskType { ASSIGNMENT, QUIZ, EXAM, PROJECT, LAB, PERSONAL };
enum class TaskStatus { PENDING, IN_PROGRESS, COMPLETED, OVERDUE };

string taskTypeToString(TaskType t);
TaskType stringToTaskType(const string& s);
string taskStatusToString(TaskStatus s);
TaskStatus stringToTaskStatus(const string& s);

// ---------------- Base Task class (Abstraction + Encapsulation) ----------------
class Task {
protected:
    int id;
    string title;
    string subject;
    TaskType type;
    int deadlineDays;      // days remaining from "today" (simulation clock)
    int workloadHours;     // estimated hours needed to finish
    int priority;           // user set base priority 1(low) - 5(high)
    TaskStatus status;

public:
    Task(int id, string title, string subject, TaskType type, int deadlineDays, int workloadHours, int priority);
    virtual ~Task() {}

    // ---- Getters ----
    int getId() const { return id; }
    string getTitle() const { return title; }
    string getSubject() const { return subject; }
    TaskType getType() const { return type; }
    int getDeadline() const { return deadlineDays; }
    int getWorkload() const { return workloadHours; }
    int getPriority() const { return priority; }
    TaskStatus getStatus() const { return status; }

    // ---- Setters ----
    void setDeadline(int d) { deadlineDays = d; }
    void setStatus(TaskStatus s) { status = s; }
    void setPriority(int p) { priority = p; }
    void reduceDeadlineByOneDay() { if (deadlineDays > -9999) deadlineDays--; }

    // ---- Polymorphic behaviour (Inheritance + Polymorphism) ----
    virtual string getTypeName() const = 0;     // pure virtual -> abstraction
    virtual double getTypeWeight() const = 0;   // how "heavy" this task type is for risk

    virtual double calculateUrgency() const;                       // based on deadline
    virtual double calculateRisk(int pendingDependencies) const;   // full risk score
    virtual void display() const;
    virtual string serialize() const;   // for file storage

    static Task* deserialize(const string& line);
};

// ---------------- Derived task types ----------------
class Assignment : public Task {
public:
    Assignment(int id, string title, string subject, int deadlineDays, int workloadHours, int priority);
    string getTypeName() const override { return "Assignment"; }
    double getTypeWeight() const override { return 1.0; }
};

class Quiz : public Task {
public:
    Quiz(int id, string title, string subject, int deadlineDays, int workloadHours, int priority);
    string getTypeName() const override { return "Quiz"; }
    double getTypeWeight() const override { return 1.2; }
};

class Exam : public Task {
public:
    Exam(int id, string title, string subject, int deadlineDays, int workloadHours, int priority);
    string getTypeName() const override { return "Exam"; }
    double getTypeWeight() const override { return 2.0; }
};

class Project : public Task {
public:
    Project(int id, string title, string subject, int deadlineDays, int workloadHours, int priority);
    string getTypeName() const override { return "Project"; }
    double getTypeWeight() const override { return 1.5; }
};

class LabWork : public Task {
public:
    LabWork(int id, string title, string subject, int deadlineDays, int workloadHours, int priority);
    string getTypeName() const override { return "Lab"; }
    double getTypeWeight() const override { return 1.1; }
};

class PersonalActivity : public Task {
public:
    PersonalActivity(int id, string title, string subject, int deadlineDays, int workloadHours, int priority);
    string getTypeName() const override { return "Personal"; }
    double getTypeWeight() const override { return 0.7; }
};

// Factory function (used by loader & menu)
Task* createTask(TaskType type, int id, string title, string subject, int deadlineDays, int workloadHours, int priority);

#endif
