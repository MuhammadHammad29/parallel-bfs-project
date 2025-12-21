// graph_utils.h
// -----------------------------------------------------------------------------
// Shared utilities for building/reading graphs and parsing simple CLI arguments.
// This header is used by both bfs_sequential.cpp and bfs_openmp.cpp to ensure
// they run on the *same inputs* for fair comparison.
// -----------------------------------------------------------------------------
//
// Graph model: undirected adjacency list (vector<vector<int>>).
// Synthetic generator: builds a random graph with approximately `avg_deg` edges
// per vertex (without self-loops, with simple dedup).
//
// CLI usage (recognized by both executables):
//   --n <int>        number of vertices (default 10000)        [ignored if --file]
//   --deg <int>      approximate average degree (default 8)     [ignored if --file]
//   --start <int>    BFS start vertex (default 0)
//   --file <path>    load undirected edge list "u v" (0-based indices)
//   --seed <uint64>  RNG seed for synthetic graph (default 42)
//
// Example (synthetic):
//   ./bfs_seq  --n 100000 --deg 8 --start 0
// Example (file):
//   ./bfs_par  --n 100000 --start 0 --file edges.txt
// -----------------------------------------------------------------------------

#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <iostream>
#include <cstdint>
#include <fstream>
using namespace std;

// Simple adjacency-list graph
using Graph = vector<vector<int>>;

// Build an undirected random graph with ~avg_deg neighbors per vertex.
inline Graph make_synthetic_graph(int n, int avg_deg, bool directed, uint64_t seed = 42) {
    Graph g(n);
    if (avg_deg < 0) avg_deg = 0;
    if (avg_deg > n - 1) avg_deg = n - 1;  // can't have more neighbors than (n-1)

    mt19937_64 rng(seed);
    uniform_int_distribution<int> dist(0, n - 1);

    for (int u = 0; u < n; ++u) {
        unordered_set<int> seen;
        while ((int)seen.size() < avg_deg) {
            int v = dist(rng);
            if (v != u) seen.insert(v); // avoid self-loop
        }
        for (int v : seen) {
            g[u].push_back(v);
            if (!directed) {
                g[v].push_back(u);
            }
        }
    }

    // Final global cleanup: guarantee sorted + unique neighbor lists for all nodes
    for (int u = 0; u < n; ++u) {
        auto& adj = g[u];
        sort(adj.begin(), adj.end());
        adj.erase(unique(adj.begin(), adj.end()), adj.end());
    }
    return g;
}


// Load an undirected graph from a simple edge list file with lines "u v".
// Assumes 0-based vertex IDs and ignores invalid pairs/out-of-range lines.
inline Graph load_edgelist(istream& in, int n) {
    Graph g(n);
    int u, v;
    while (in >> u >> v) {
        if (u >= 0 && u < n && v >= 0 && v < n && u != v) {
            g[u].push_back(v);
            g[v].push_back(u);
        }
    }
    for (int u = 0; u < n; ++u) {
        sort(g[u].begin(), g[u].end());
        g[u].erase(unique(g[u].begin(), g[u].end()), g[u].end());
    }
    return g;
}

// Print short usage help to stderr (used when arguments are invalid).
inline void usage(const char* prog) {
    cerr << "Usage:\n"
         << "  " << prog << " --n 100000 --deg 8 --start 0 [--seed 42]\n"
         << "  " << prog << " --n 100000 --start 0 --file input.txt\n";
}

// Minimal CLI parser shared by both binaries.
// Validate arguments
inline bool parse_args(int argc, char** argv, int& n, int& deg, int& start,
                       string& file, uint64_t& seed, int& iters, bool& directed) {
    n = 10000; deg = 8; start = 0; file = ""; seed = 42; iters = 1, directed = false;

    for (int i = 1; i < argc; ++i) {
        string a = argv[i];
        auto need = [&](int j){ return j + 1 < argc; };
        if      (a == "--n"     && need(i)) n    = atoi(argv[++i]);
        else if (a == "--deg"   && need(i)) deg  = atoi(argv[++i]);
        else if (a == "--start" && need(i)) start= atoi(argv[++i]);
        else if (a == "--file"  && need(i)) file = argv[++i];
        else if (a == "--seed"  && need(i)) seed = strtoull(argv[++i], nullptr, 10);
        else if (a == "--iters" && need(i)) iters = atoi(argv[++i]);
        else if (a == "--directed") directed = true;
        else { usage(argv[0]); return false; }
    }

    if (n <= 0)                 { cerr << "Invalid --n\n"; return false; }
    if (deg < 0)                { cerr << "Invalid --deg\n"; return false; }
    if (start < 0 || start >= n){ cerr << "Invalid --start\n"; return false; }
    if (iters <= 0) { cerr << "Invalid --iters\n"; return false; }
    return true;
}
