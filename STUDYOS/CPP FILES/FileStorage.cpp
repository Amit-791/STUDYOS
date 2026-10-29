#include "FileStorage.h"
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

bool FileStorage::saveTasks(const string& filename, const TaskManager& mgr) {
    ofstream out(filename);
    if (!out.is_open()) return false;

    for (Task* t : mgr.getAllTasks()) {
        out << t->serialize() << "\n";
    }
    out.close();
    return true;
}

bool FileStorage::loadTasks(const string& filename, TaskManager& mgr) {
    ifstream in(filename);
    if (!in.is_open()) return false;

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        Task* t = Task::deserialize(line);
        if (t) mgr.insertLoadedTask(t);
    }
    in.close();
    return true;
}

bool FileStorage::saveDependencies(const string& filename, const TaskManager& mgr) {
    ofstream out(filename);
    if (!out.is_open()) return false;

    for (auto& e : mgr.getAllDependencyEdges()) {
        out << e.first << "," << e.second << "\n";
    }
    out.close();
    return true;
}

bool FileStorage::loadDependencies(const string& filename, TaskManager& mgr) {
    ifstream in(filename);
    if (!in.is_open()) return false;

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string a, b;
        getline(ss, a, ',');
        getline(ss, b, ',');
        if (a.empty() || b.empty()) continue;
        mgr.addDependencyRaw(stoi(a), stoi(b));
    }
    in.close();
    return true;
}
