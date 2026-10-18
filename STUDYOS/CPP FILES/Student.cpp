#include "Student.h"
#include <iostream>
#include <algorithm>
using namespace std;

Student::Student(int id, string name, int dailyStudyHours)
    : id(id), name(move(name)), dailyStudyHours(dailyStudyHours) {}

void Student::assignTask(int taskId) {
    taskIds.push_back(taskId);
}

void Student::removeTask(int taskId) {
    taskIds.erase(remove(taskIds.begin(), taskIds.end(), taskId), taskIds.end());
}

void Student::display() const {
    cout << "Student #" << id << " - " << name
         << " | Daily study hours available: " << dailyStudyHours
         << " | Tasks assigned: " << taskIds.size() << "\n";
}
