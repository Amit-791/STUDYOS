// STUDYOS web version.
// The logic below mirrors the C++ program (Task, Graph, TaskManager).
// If you change a formula in the C++ code, change it here too.

const TYPES = {
  ASSIGNMENT: { name: "Assignment", weight: 1.0 },
  QUIZ:       { name: "Quiz",       weight: 1.2 },
  EXAM:       { name: "Exam",       weight: 2.0 },
  PROJECT:    { name: "Project",    weight: 1.5 },
  LAB:        { name: "Lab",        weight: 1.1 },
  PERSONAL:   { name: "Personal",   weight: 0.7 },
};

const STORAGE_KEY = "studyos-data";

// ---------------- Data ----------------
let state = { tasks: [], edges: [], nextId: 1 };   // edge [u, v]: u must finish before v

function addTask(type, title, subject, deadline, workload, priority) {
  const id = state.nextId++;
  state.tasks.push({ id, type, title, subject, deadline, workload, priority, status: "PENDING" });
  return id;
}

const getTask = (id) => state.tasks.find((t) => t.id === id);
const isDone = (t) => t.status === "COMPLETED";

// ---------------- Dependency graph ----------------
const dependentsOf = (id) => state.edges.filter((e) => e[0] === id).map((e) => e[1]);

// Kahn's algorithm. Returns the order, or null if there is a cycle.
function topologicalSort() {
  const indegree = new Map(state.tasks.map((t) => [t.id, 0]));
  state.edges.forEach(([, v]) => indegree.set(v, indegree.get(v) + 1));

  const queue = state.tasks.filter((t) => indegree.get(t.id) === 0).map((t) => t.id);
  const order = [];
  while (queue.length) {
    const cur = queue.shift();
    order.push(cur);
    for (const next of dependentsOf(cur)) {
      indegree.set(next, indegree.get(next) - 1);
      if (indegree.get(next) === 0) queue.push(next);
    }
  }
  return order.length === state.tasks.length ? order : null;
}

// Adds the edge, then undoes it if it creates a cycle.
function addDependency(prerequisite, dependent) {
  if (!getTask(prerequisite) || !getTask(dependent)) return false;
  state.edges.push([prerequisite, dependent]);
  if (topologicalSort() === null) {
    state.edges = state.edges.filter((e) => !(e[0] === prerequisite && e[1] === dependent));
    return false;
  }
  return true;
}

// BFS: every task reachable from startId, excluding itself.
function impactedTasks(startId) {
  const visited = new Set([startId]);
  const queue = [startId];
  const result = [];
  while (queue.length) {
    const cur = queue.shift();
    if (cur !== startId) result.push(cur);
    for (const next of dependentsOf(cur)) {
      if (!visited.has(next)) {
        visited.add(next);
        queue.push(next);
      }
    }
  }
  return result;
}

// ---------------- Risk, sorting, scheduling ----------------
const pendingDependencies = (id) =>
  state.edges.filter(([u, v]) => v === id && getTask(u) && !isDone(getTask(u))).length;

const urgency = (t) => (t.deadline <= 0 ? 100 : 100 / (t.deadline + 1));

function risk(t) {
  let score = urgency(t) * TYPES[t.type].weight;
  score += t.priority * 2;
  score += t.workload * 0.5;
  score += pendingDependencies(t.id) * 3;
  if (t.status === "OVERDUE") score += 50;
  return score;
}

// Same merge sort as Utils.h. comesFirst(a, b) is true if a goes before b.
function mergeSort(items, comesFirst) {
  if (items.length <= 1) return items.slice();
  const mid = Math.floor((items.length - 1) / 2) + 1;
  const left = mergeSort(items.slice(0, mid), comesFirst);
  const right = mergeSort(items.slice(mid), comesFirst);
  const out = [];
  let i = 0, j = 0;
  while (i < left.length && j < right.length) {
    out.push(comesFirst(left[i], right[j]) ? left[i++] : right[j++]);
  }
  return out.concat(left.slice(i), right.slice(j));
}

const COMPARATORS = {
  id:       (a, b) => a.id < b.id,
  deadline: (a, b) => a.deadline < b.deadline,
  priority: (a, b) => a.priority > b.priority,
  workload: (a, b) => a.workload < b.workload,
  risk:     (a, b) => risk(a) > risk(b),
};

const sortTasks = (key) => mergeSort(state.tasks, COMPARATORS[key]);

// The C++ program uses a priority queue; sorting gives the same top-k scores.
const topUrgent = (k) => sortTasks("risk").filter((t) => !isDone(t)).slice(0, Math.max(0, k));

// Greedy: highest risk per hour first, take each task that still fits.
function dailyPlan(availableHours) {
  const candidates = state.tasks.filter((t) => !isDone(t) && pendingDependencies(t.id) === 0);
  const ratio = (t) => risk(t) / Math.max(1, t.workload);
  const plan = [];
  let used = 0;
  for (const t of mergeSort(candidates, (a, b) => ratio(a) > ratio(b))) {
    if (used + t.workload <= availableHours) {
      plan.push(t);
      used += t.workload;
    }
  }
  return plan;
}

// ---------------- Storage (replaces FileStorage) ----------------
function save() {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(state));
  } catch (err) {
    showMessage("Could not save in this browser. Changes will be lost on reload.", true);
  }
}

function load() {
  try {
    const data = JSON.parse(localStorage.getItem(STORAGE_KEY));
    if (data && Array.isArray(data.tasks) && Array.isArray(data.edges)) state = data;
  } catch (err) {
    // Nothing usable saved: start fresh.
  }
}

// ---------------- Page ----------------
const $ = (selector) => document.querySelector(selector);

// Builds an element. Text is added as text, so task titles cannot inject HTML.
function el(tag, content, className) {
  const node = document.createElement(tag);
  if (className) node.className = className;
  [].concat(content ?? []).forEach((c) => node.append(c instanceof Node ? c : String(c)));
  return node;
}

function table(headings, rows) {
  const head = el("thead", el("tr", headings.map((h) => el("th", h))));
  const body = el("tbody", rows.map((r) => el("tr", r.cells.map((c) => el("td", c)), r.className)));
  return el("div", el("table", [head, body]), "table-wrap");
}

const show = (selector, ...nodes) => $(selector).replaceChildren(...nodes);
const empty = (text) => el("p", text, "empty");
const label = (t) => `#${t.id} ${t.title}`;

function showMessage(text, isError = false) {
  const box = $("#message");
  box.textContent = text;
  box.className = isError ? "message error" : "message";
  box.hidden = false;
}

function renderTasks() {
  if (!state.tasks.length) return show("#tasks-out", empty("No tasks yet. Add one, or load the sample."));
  const rows = sortTasks($("#sort-by").value).map((t) => {
    let action = "";
    if (!isDone(t)) {
      action = el("button", "Mark done", "small secondary");
      action.type = "button";
      action.addEventListener("click", () => {
        t.status = "COMPLETED";
        update(`Task #${t.id} marked completed.`);
      });
    }
    return {
      className: isDone(t) ? "done" : "",
      cells: [t.id, TYPES[t.type].name, t.title, t.subject, t.deadline,
              `${t.workload}h`, t.priority, t.status, action],
    };
  });
  show("#tasks-out", table(
    ["ID", "Type", "Title", "Subject", "Days left", "Workload", "Priority", "Status", ""], rows));
}

function renderPlan() {
  const hours = Number($("#hours").value) || 0;
  const plan = dailyPlan(hours);
  if (!plan.length) return show("#plan-out", empty("Nothing schedulable right now."));
  const used = plan.reduce((sum, t) => sum + t.workload, 0);
  show("#plan-out",
    el("ol", plan.map((t) =>
      el("li", `[${TYPES[t.type].name}] ${t.title} (${t.workload}h, priority ${t.priority})`))),
    el("p", `Total hours planned: ${used} / ${hours}`));
}

function renderRiskTable(selector, tasks, emptyText) {
  if (!tasks.length) return show(selector, empty(emptyText));
  show(selector, table(
    ["ID", "Type", "Title", "Days left", "Pending prerequisites", "Risk score"],
    tasks.map((t) => ({
      cells: [t.id, TYPES[t.type].name, t.title, t.deadline,
              pendingDependencies(t.id), risk(t).toFixed(2)],
    }))));
}

function renderOrder() {
  const order = topologicalSort();
  if (!state.tasks.length) show("#order-out", empty("No tasks yet."));
  else if (!order) show("#order-out", empty("Cycle detected in dependencies. No valid order exists."));
  else show("#order-out", el("ol", order.map((id) => {
    const t = getTask(id);
    return el("li", `[${TYPES[t.type].name}] ${label(t)}`);
  })));

  if (!state.edges.length) return show("#deps-out", empty("No dependencies yet."));
  show("#deps-out", el("ul", state.edges.map(([u, v]) =>
    el("li", `${label(getTask(u))} → ${label(getTask(v))}`))));
}

function renderDelay() {
  const t = getTask(Number($("#delay-task").value));
  if (!t) return show("#delay-out", empty("Add a task to run a simulation."));
  const days = Number($("#delay-days").value) || 0;
  const summary = el("p",
    `Delaying ${label(t)} by ${days} day(s) leaves ${t.deadline - days} day(s) before its deadline.`);
  const impacted = impactedTasks(t.id).map(getTask);
  if (!impacted.length) return show("#delay-out", summary, empty("No dependent tasks are affected."));
  show("#delay-out", summary, table(
    ["ID", "Title", "Deadline", "New slack (days)", "Warning"],
    impacted.map((d) => {
      const slack = d.deadline - days;
      let warning = "";
      if (slack < 0) warning = el("span", "Deadline will be missed", "tag-danger");
      else if (slack <= 1) warning = el("span", "High risk, very tight", "tag-warn");
      return { cells: [d.id, d.title, d.deadline, slack, warning] };
    })));
}

function fillTaskSelect(select) {
  const previous = select.value;
  select.replaceChildren(...state.tasks.map((t) => {
    const option = el("option", label(t));
    option.value = t.id;
    return option;
  }));
  if (getTask(Number(previous))) select.value = previous;
}

function render() {
  const depForm = $("#dep-form");
  fillTaskSelect(depForm.elements.prerequisite);
  fillTaskSelect(depForm.elements.dependent);
  fillTaskSelect($("#delay-task"));
  renderTasks();
  renderPlan();
  renderRiskTable("#urgent-out", topUrgent(Number($("#top-k").value) || 0), "No pending tasks.");
  renderRiskTable("#risk-out", sortTasks("risk").filter((t) => !isDone(t)), "No pending tasks.");
  renderOrder();
  renderDelay();
}

// Re-draw after a change to the data, then save.
function update(message) {
  render();
  showMessage(message);
  save();
}

function loadSample() {
  state = { tasks: [], edges: [], nextId: 1 };
  const notes = addTask("PERSONAL", "Revise lecture notes", "DSA", 2, 2, 3);
  const sheet = addTask("ASSIGNMENT", "Graph problem sheet", "DSA", 3, 3, 4);
  const exam = addTask("EXAM", "End-semester exam", "DSA", 8, 6, 5);
  const lab = addTask("LAB", "Lab report", "Electronics", 1, 2, 3);
  const project = addTask("PROJECT", "Project demo", "OOP", 5, 4, 4);
  addDependency(notes, sheet);
  addDependency(sheet, exam);
  addDependency(lab, project);
}

function init() {
  load();

  $("#task-form").addEventListener("submit", (event) => {
    event.preventDefault();
    const f = event.target.elements;
    const title = f.namedItem("title").value.trim();
    const subject = f.subject.value.trim();
    if (!title || !subject) return showMessage("Title and subject cannot be blank.", true);
    const id = addTask(f.type.value, title, subject,
      Number(f.deadline.value), Number(f.workload.value), Number(f.priority.value));
    f.namedItem("title").value = "";
    update(`Task added with ID #${id}.`);
  });

  $("#dep-form").addEventListener("submit", (event) => {
    event.preventDefault();
    const f = event.target.elements;
    const a = Number(f.prerequisite.value);
    const b = Number(f.dependent.value);
    if (addDependency(a, b)) update(`Dependency added: Task #${a} → Task #${b}.`);
    else showMessage("Failed to add dependency (missing task or it would create a cycle).", true);
  });

  document.querySelectorAll(".tabs button").forEach((button) => {
    button.addEventListener("click", () => {
      document.querySelectorAll(".tabs button").forEach((b) => b.classList.toggle("active", b === button));
      document.querySelectorAll(".panel").forEach((p) => {
        p.hidden = p.id !== `tab-${button.dataset.tab}`;
      });
    });
  });

  ["#sort-by", "#hours", "#top-k", "#delay-task", "#delay-days"].forEach((selector) => {
    $(selector).addEventListener("input", render);
  });

  $("#sample-btn").addEventListener("click", () => {
    if (state.tasks.length && !confirm("Replace your current tasks with sample data?")) return;
    loadSample();
    update("Sample data loaded.");
  });

  $("#clear-btn").addEventListener("click", () => {
    if (!confirm("Delete all tasks and dependencies?")) return;
    state = { tasks: [], edges: [], nextId: 1 };
    update("All data cleared.");
  });

  render();
}

init();
