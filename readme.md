## 📘 Overview
This repository implements both **Sequential** and **Parallel (OpenMP)** versions of the **Breadth-First Search (BFS)** algorithm.  
The project demonstrates how a classic graph traversal algorithm can be parallelized to improve performance, while exploring core parallel programming challenges like synchronization, data dependency, and load balancing.

---

## 🎯 Objectives
- Develop a **functional sequential BFS** as a correctness baseline.  
- Implement a **partially working parallel BFS** using **OpenMP**.  
- Record and analyze **execution times**, **speedup**, and **scaling** behavior.  
- Identify performance bottlenecks and discuss improvements.

---

## 🧩 Project Structure
📁 project_bfs/
├── graph_utils.h # Shared graph generator and argument parser 
├── bfs_sequential.cpp # Sequential BFS baseline (timed) 
├── bfs_openmp.cpp # Parallel BFS (OpenMP level-synchronous) 
├── results.txt # Mid-term performance results 
└── README.md # Project overview and usage instructions 

---

## ⚙️ Compilation Instructions

### 🔹 Windows (PowerShell)
powershell
g++ -O3 -std=c++17 bfs_sequential.cpp -o bfs_seq 
g++ -O3 -std=c++17 -fopenmp bfs_openmp.cpp -o bfs_par 

Running the Programs: 
Sequential BFS: 
.\bfs_seq.exe --n 10000 --deg 8 --start 0 
Parallel BFS (OpenMP) 
$Env:OMP_NUM_THREADS = 4
.\bfs_par.exe --n 200000 --deg 8 --start 0

Example Output: 
Seq_time_s=0.026000 
Par_time_s=0.019000 
Speedup=1.368418 
Level_check=OK 
Visited_seq=200000 Visited_par=200000 

Authors: Muhammad Hammad, Mustafa Haider, Noor Ul Haq, Faraz Ali, Nasir Khan 
Course: Parallel & Distributed Computing 
Institution: Szabist 
Instructor: Dr. Syed Samar Yazdani 
Semester: Fall 2025 


