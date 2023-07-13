#include "utils.h"

struct UnionFind { // The range of node number is u 0 v n-1
  std::vector<int> rank, parents;
  UnionFind() {}
  UnionFind(int n) { // make n trees.
    rank.resize(n, 0);
    parents.resize(n, 0);
    for (int i = 0; i < n; i++) {
      makeTree(i);
    }
  }
  void makeTree(int x) {
    parents[x] = x; // the parent of x is x
    rank[x] = 0;
  }
  bool isSame(int x, int y) { return findRoot(x) == findRoot(y); }
  void unite(int x, int y) {
    x = findRoot(x);
    y = findRoot(y);
    if (rank[x] > rank[y]) {
      parents[y] = x;
    } else {
      parents[x] = y;
      if (rank[x] == rank[y]) {
        rank[y]++;
      }
    }
  }
  int findRoot(int x) {
    if (x != parents[x])
      parents[x] = findRoot(parents[x]);
    return parents[x];
  }
};

struct Edge {
  long long u;
  long long v;
  long long cost;
};
inline bool comp_e(const Edge &e1, const Edge &e2) { return e1.cost < e2.cost; }

struct Kruskal {
  UnionFind uft;
  long long sum; // 最小全域木の重みの総和
  std::vector<Edge> *edges;
  int V;
  Kruskal(std::vector<Edge> *edges_, int V_) : edges(edges_), V(V_) { init(); }
  void init() {
    sort(edges->begin(), edges->end(), comp_e); // 辺の重みでソート
    uft = UnionFind(V);
    sum = 0;
    int num_edges = edges->size();
    for (int i = 0; i < num_edges; i++) {
      Edge e = edges->at(i);
      if (!uft.isSame(e.u, e.v)) { // 閉路にならなければ加える
        uft.unite(e.u, e.v);
        sum += e.cost;
      }
    }
  }
};

inline int mst_cost(std::vector<Edge> *edges, int N) {
  Kruskal krs(edges, N);
  return krs.sum;
}
