# STUDYOS

**Intelligent Academic Planning System**

STUDYOS helps a student decide what to study next. It tracks tasks and their prerequisites, scores each task by risk, and builds a daily study plan.

The main project is a **C++ console application** built for data structures and algorithms. The `docs` folder holds an extra **web interface**.

## Features

- Add assignments, quizzes, exams, projects, labs and personal tasks
- Add dependencies between tasks, with cycle detection
- View a valid execution order
- View the most urgent tasks
- Sort tasks by deadline, priority, workload or risk
- Generate a daily study plan
- Simulate the effect of delaying a task
- Mark tasks as completed
- View a risk analysis report
- Save and load data

## Data structures and algorithms (C++)

| Concept | Used for |
|---|---|
| Hash table (`unordered_map`) | Looking up tasks by ID |
| Directed graph (adjacency list) | Task dependencies |
| Topological sort (Kahn's algorithm) | Execution order and cycle detection |
| Breadth-first search | Finding tasks affected by a delay |
| Priority queue (heap) | Top urgent tasks |
| Merge sort | Sorting tasks |
| Greedy algorithm | Daily study plan |

Object-oriented design: an abstract `Task` class with six derived task types.

## Risk score

```
urgency = 100                    if days left <= 0
urgency = 100 / (days left + 1)  otherwise

risk = urgency x type weight
     + priority x 2
     + workload hours x 0.5
     + pending prerequisites x 3
```

| Type | Weight |
|---|---|
| Exam | 2.0 |
| Project | 1.5 |
| Quiz | 1.2 |
| Lab | 1.1 |
| Assignment | 1.0 |
| Personal | 0.7 |

## Project structure

```
STUDYOS/
  CPP FILES/       source files
  HEADER FILES/    header files
docs/
  index.html       web page
  style.css        design
  script.js        logic for the web version
README.md
```

## Run the C++ program

You need a compiler that supports C++17, such as `g++`. From the repository folder:

```bash
cd STUDYOS
g++ -std=c++17 -I"HEADER FILES" "CPP FILES/main.cpp" "CPP FILES/Task.cpp" "CPP FILES/TaskManager.cpp" "CPP FILES/Graph.cpp" "CPP FILES/FileStorage.cpp" "CPP FILES/Student.cpp" -o studyos
./studyos
```

On Windows PowerShell, run `.\studyos.exe` for the last step.

Choose **Save & Exit** (option 11) to keep your changes.

## Run the web version

Open `docs/index.html` in a browser.

The web version does not run the C++ code. It reimplements the main logic in JavaScript, and some data structures differ. It saves data in the browser, separately from the C++ program. It was built with AI assistance.

## Team

- [Amit-791](https://github.com/Amit-791)
- [kk-87](https://github.com/kk-87)
