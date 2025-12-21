// bfs_sequential.cpp
// -----------------------------------------------------------------------------
// Sequential BFS baseline.
// - Uses an adjacency list (vector<vector<int>>) for simplicity and good cache
//   locality compared to linked lists.
// - NO printing inside the BFS loop (I/O would dominate runtime).
// - Returns an optional 'level' array to enable correctness checks against
//   the parallel version.
// - Prints total time and visited count for benchmarking.
// Complexity: O(V + E)
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
using namespace std;

// Standard queue-based BFS. If level_out is provided, we fill each node's level
// (distance in edges from the start node; -1 means unreachable).
static vector<int> bfs_seq(const Graph& g, int s, vector<int>* level_out = nullptr) {
    const int n = (int)g.size();
    vector<char> vis(n, 0); //nodes visited
    vector<int> q;      q.reserve(n); //BFS queue
    vector<int> order;  order.reserve(n); //order of visitation
    vector<int> level(n, -1); //level of each node

    vis[s] = 1; level[s] = 0; q.push_back(s); //initialize start node

    // Typical BFS loop using an index as queue head (faster than std::queue here).
    for (size_t h = 0; h < q.size(); ++h) {
        int u = q[h]; // current node
        order.push_back(u); // record visitation order
        for (int v : g[u]) { // explore neighbors
            if (!vis[v]) {
                vis[v] = 1;
                level[v] = level[u] + 1; // set level for neighbor(parent level + 1)
                q.push_back(v); // enqueue neighbor
            }
        }
    }

    if (level_out) *level_out = std::move(level); 
    return order;
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Parse shared CLI options
    int n, deg, start;bool directed; string file; uint64_t seed; int iters;
    if (!parse_args(argc, argv, n, deg, start, file, seed, iters,directed)) return 1;

    // Build or load the graph once
    Graph g;
    if (!file.empty()) {
        ifstream fin(file);
        if (!fin) { cerr << "Failed to open " << file << "\n"; return 1; }
        g = load_edgelist(fin, n);
    } else {
        g = make_synthetic_graph(n, deg, directed, seed);
    }

    ofstream fout("edges.txt");
    for (int u = 0; u < g.size(); u++) {
    for (int v : g[u]) {
        if (u < v)   // avoid duplicate edges in undirected graph
            fout << u << " " << v << "\n";
        }
    }
    fout.close();


    // Time only the BFS computation
    auto t0 = chrono::steady_clock::now();
    vector<int> lvl_seq;
    vector<int> ord;

    for (int k = 0; k < iters; ++k) {
    ord = bfs_seq(g, start, &lvl_seq);
    }

    auto t1 = chrono::steady_clock::now();


    //metrics 
    chrono::duration<double> dt = t1 - t0; 
    cout.setf(std::ios::fixed); cout << setprecision(6);
    cout << "Seq_time_s=" << dt.count() << "\n";
    cout << "Iters=" << iters << "\n";
    cout << "Avg_time_s=" << (dt.count() / iters) << "\n";
    cout << "Visited_count=" << ord.size() << "\n";
    cout << "Start=" << start << " N=" << n << "\n";
    return 0;
}
