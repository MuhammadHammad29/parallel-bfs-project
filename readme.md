```markdown
## 📘 Overview
This repository implements both **Sequential** and **Parallel (OpenMP)** versions of the **Breadth-First Search (BFS)** algorithm.  
The project demonstrates how a classic graph traversal algorithm can be parallelized to improve performance, while exploring core parallel programming challenges like synchronization, data dependency,load balancing,and graph structure effects (directed vs undirected).

---

## 🎯 Objectives
- Develop a **functional sequential BFS** as a correctness baseline.  
- Implement a **parallel BFS** using **OpenMP** (level-synchronous frontier expansion). 
- REvaluate **scalability** across graph sizes, thread counts, and graph types.  
- Analyze performance bottlenecks in **synthetic**, **real-world**, and **directed** graphs.

---

## 🧩 Project Structure
```
PROJECT_BFS/
├─ bfs_openmp.cpp          # Parallel BFS (OpenMP, undirected + directed)
├─ bfs_sequential.cpp      # Sequential BFS baseline
├─ graph_utils.h           # Graph generation, file loading, CLI parsing
├─ com-youtube.ungraph.txt # Real YouTube SNAP dataset (undirected)
├─ edges.txt               # Generated edge list (from synthetic graph)
├─ graph.dot               # GraphViz DOT file (visualization)
├─ graph.png               # Rendered graph image
├─ results.txt             # Final performance results (updated)
└─ readme.md               # Project documentation (updated)
````

---

## ⚙️ Compilation Instructions

### 🔹 Windows (PowerShell)

**Compile:**
```powershell
# Sequential
g++ -O3 -std=c++17 bfs_sequential.cpp -o bfs_seq.exe

# Parallel (OpenMP)
g++ -O3 -std=c++17 -fopenmp bfs_openmp.cpp -o bfs_par.exe
````

▶️ Usage Instructions

```powershell
# Sequential BFS
.\bfs_seq.exe --n 10000 --deg 8 --start 0

# Parallel BFS (Undirected Synthetic Graph)(OpenMP) – set threads then run
$Env:OMP_NUM_THREADS = 8
.\bfs_par.exe --n 200000 --deg 8 --start 0

# Parallel BFS (Directed Synthetic Graph)(OpenMP) – set threads then run
$Env:OMP_NUM_THREADS = 8
.\bfs_par.exe --n 200000 --deg 8 --start 0 --directed

# Parallel BFS (Real Graph from File)(OpenMP) – set threads then run
$Env:OMP_NUM_THREADS = 8
.\bfs_par.exe --n 1157828 --start 1 --file com-youtube.ungraph.txt

# Stable Benchmarking with Iterations
# To reduce timing noise on fast runs, multiple BFS iterations can be executed using --iters:
$Env:OMP_NUM_THREADS = 8
.\bfs_par.exe --n 1200000 --deg 8 --start 0 --iters 20
```

**Example Output:**

```
Seq_time_s=<total sequential time>
Par_time_s=<total parallel time>
Iters=<number of BFS repetitions>
Speedup=<Seq_time / Par_time>
Level_check=OK
Visited_seq=<nodes visited>
Visited_par=<nodes visited>
```
Level_check=OK confirms correctness by matching BFS level arrays between sequential and parallel executions.

📈 Results Summary:
Detailed performance results are documented in results.txt, covering:
Small, medium, and large synthetic graphs
Real social network graph (YouTube SNAP dataset)
Directed synthetic graphs
Speedup trends from 1 to 8 threads
---

## 👨‍🏫 Authors

**Muhammad Hammad**, **Mustafa Haider**, **Noor Ul Haq**, **Faraz Ali**, **Nasir Khan**
**Course:** Parallel & Distributed Computing
**Institution:** SZABIST
**Instructor:** Dr. Syed Samar Yazdani
**Semester:** Fall 2025

``````



