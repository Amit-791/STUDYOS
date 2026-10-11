#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>
using namespace std;

class Student {
private:
    int id;
    string name;
    int dailyStudyHours;      // available hours per day for study
    vector<int> taskIds;       // ids of tasks assigned to this student

public:
    Student(int id, string name, int dailyStudyHours);

    int getId() const { return id; }
    string getName() const { return name; }
    int getDailyStudyHours() const { return dailyStudyHours; }
    void setDailyStudyHours(int hrs) { dailyStudyHours = hrs; }

    void assignTask(int taskId);
    void removeTask(int taskId);
    const vector<int>& getTaskIds() const { return taskIds; }

    void display() const;
};

#endif
