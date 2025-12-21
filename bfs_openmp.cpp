// bfs_openmp.cpp
// -----------------------------------------------------------------------------
// Partially working parallel BFS using OpenMP (level-synchronous).
// - Each BFS "frontier" (the current level's nodes) is expanded in parallel.
// - To avoid contention, each thread collects discovered nodes in a local
//   buffer; we merge all buffers at the end of the level.
// - Race-free 'visited' using atomic test-and-set (exchange from 0->1).
// - Prints sequential time, parallel time, speedup, and a level-equality check.
// This meets the mid-term requirement: a parallel version that correctly
// handles small/medium data and shows preliminary performance results.
// -----------------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <chrono>
#include <atomic>
#include <iomanip>
#include <fstream>     // needed for file input
#include "graph_utils.h"
#ifdef _OPENMP
#include <omp.h>
#endif
using namespace std;

// Reuse the sequential BFS to (1) compare times and (2) verify correctness by
// comparing the level arrays (where nodes are reachable).
static vector<int> bfs_seq(const Graph& g, int s, vector<int>* level_out = nullptr) {
    const int n = (int)g.size();
    vector<char> vis(n, 0);
    vector<int> q;      q.reserve(n);
    vector<int> order;  order.reserve(n);
    vector<int> level(n, -1);

    vis[s] = 1; level[s] = 0; q.push_back(s);
    for (size_t h = 0; h < q.size(); ++h) {
        int u = q[h];
        order.push_back(u);
        for (int v : g[u]) {
            if (!vis[v]) {
                vis[v] = 1;
                level[v] = level[u] + 1;
                q.push_back(v);
            }
        }
    }
    if (level_out) *level_out = std::move(level);
    return order;
}

// Level-synchronous parallel BFS:
// - 'frontier' contains current-level nodes.
// - Threads expand neighbors of nodes in 'frontier' concurrently.
// - 'visited[v].exchange(1)' returns previous value; only the first thread that
//   flips from 0 to 1 enqueues v into its local buffer.
// - After the parallel region, we merge all per-thread buffers to form next level.
static vector<int> bfs_openmp_level(const Graph& g, int s, vector<int>* level_out = nullptr) {
    const int n = (int)g.size();
    vector<atomic<uint8_t>> visited(n); // atomic visited flagged 0 or 1
    for (int i = 0; i < n; ++i) visited[i].store(0, memory_order_relaxed);

    vector<int> level(n, -1); // level of each node (-1 means unvisited)
    vector<int> frontier; frontier.reserve(1024); // current level's frontier
    vector<int> order;    order.reserve(n);   // order of visitation

    visited[s].store(1, memory_order_relaxed);
    level[s] = 0; 
    frontier.push_back(s);
    int curr_level = 0;

    while (!frontier.empty()) {
        // record traversal order 
        order.insert(order.end(), frontier.begin(), frontier.end());

        // per-thread buffers to avoid pushing into a shared vector
        int P = 1;
        #ifdef _OPENMP
        P = omp_get_max_threads();
        #endif
        vector<vector<int>> tls(P); // thread-local storage for next frontier
        for (int t = 0; t < P; ++t) tls[t].reserve(frontier.size() / (P + 1) + 16); 

        // Parallel expansion of the current frontier
        #pragma omp parallel
        {
            int tid = 0;
            #ifdef _OPENMP
            tid = omp_get_thread_num();
            #endif
            auto& out = tls[tid];

            #pragma omp for schedule(dynamic, 512) // dynamic scheduling for load balance
            for (int i = 0; i < (int)frontier.size(); ++i) { // for each node in frontier
                int u = frontier[i]; // current node
                for (int v : g[u]) { // explore neighbors
                    // Atomic test-and-set: only first discoverer enqueues v
                    uint8_t was = visited[v].exchange(1, memory_order_relaxed); // returns previous value 
                    if (!was) {
                        level[v] = curr_level + 1; // all writers would assign same value
                        out.push_back(v); // enqueue into thread-local buffer
                    }
                }
            }
        }

        // Merge thread-local buffers into the next frontier
        size_t total = 0; for (auto& v : tls) total += v.size();
        vector<int> next; next.reserve(total);
        for (auto& v : tls) next.insert(next.end(), v.begin(), v.end());

        frontier.swap(next);
        ++curr_level;
    }

    if (level_out) *level_out = std::move(level);
    return order;
}

// Small wall-clock helper that uses omp_get_wtime() when available.
static double wall() {
    #ifdef _OPENMP
    return omp_get_wtime();
    #else
    using clk = chrono::steady_clock; static auto t0 = clk::now();
    return chrono::duration<double>(clk::now() - t0).count();
    #endif
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Parse shared CLI options
    int n, deg, start; string file; uint64_t seed;
    if (!parse_args(argc, argv, n, deg, start, file, seed)) return 1;

    // Build or load graph once
    Graph g;
    if (!file.empty()) {
        ifstream fin(file);
        if (!fin) { cerr << "Failed to open " << file << "\n"; return 1; }
        g = load_edgelist(fin, n);
    } else {
        g = make_synthetic_graph(n, deg, seed);
    }

    // Baseline sequential run (also used for correctness checking)
    vector<int> lvl_seq, lvl_par;

    double t0 = wall();
    auto seq_order = bfs_seq(g, start, &lvl_seq);
    double t1 = wall();

    // Parallel OpenMP BFS
    double t2 = wall();
    auto par_order = bfs_openmp_level(g, start, &lvl_par);
    double t3 = wall();

    // Verify levels match where nodes are reachable in both runs.
    bool ok = true;
    for (int i = 0; i < n; ++i) {
        if (lvl_seq[i] != -1 && lvl_par[i] != -1 && lvl_seq[i] != lvl_par[i]) {
            ok = false; break;
        }
    }

    // metrics 
    cout.setf(std::ios::fixed); cout << setprecision(6);
    cout << "Seq_time_s=" << (t1 - t0) << "\n";
    cout << "Par_time_s=" << (t3 - t2) << "\n";
    cout << "Speedup="   << ((t3 - t2) > 0 ? (t1 - t0) / (t3 - t2) : 1.0) << "\n";
    cout << "Level_check=" << (ok ? "OK" : "MISMATCH") << "\n";
    cout << "Visited_seq=" << seq_order.size()
         << " Visited_par=" << par_order.size() << "\n";
    return 0;
}
