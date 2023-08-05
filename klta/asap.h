#pragma once
#include "covering_search.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

inline std::vector<std::vector<int>>
get_asaplookup(std::vector<std::vector<std::pair<int, int>>> &graph) {
  int N = graph.size();
  std::vector<std::vector<int>> asaplookup(N, std::vector<int>(N, MAX_DIST));
  for (int i = 0; i < N; i++) {
    std::priority_queue<std::pair<int, int>> que;
    std::vector<std::vector<bool>> seen(N, std::vector<bool>(N, false));
    que.push({0, i});
    asaplookup[i][i] = 0;

    while (!que.empty()) {
      int v = que.top().second;
      que.pop();

      if (seen[i][v]) {
        continue;
      }
      seen[i][v] = true;

      int j, c_vj;
      for (const std::pair<int, int> &e : graph[v]) {
        j = e.first;
        c_vj = e.second;
        if ((i == j) || c_vj == MAX_DIST) {
          continue;
        }
        if (asaplookup[i][j] > asaplookup[i][v] + c_vj) {
          asaplookup[i][j] = asaplookup[i][v] + c_vj;
          que.push({-1 * asaplookup[i][j], j});
        }
      }
    }
  }

  return asaplookup;
}
