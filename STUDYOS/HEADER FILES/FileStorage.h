 #ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include "TaskManager.h"
#include <string>
using namespace std;

class FileStorage {
public:
    static bool saveTasks(const string& filename, const TaskManager& mgr);
    static bool loadTasks(const string& filename, TaskManager& mgr);

    static bool saveDependencies(const string& filename, const TaskManager& mgr);
    static bool loadDependencies(const string& filename, TaskManager& mgr);
};

#endif
