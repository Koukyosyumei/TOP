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
get_asaplookup(std::vector<std::vector<int>> &graph) {
  int N = graph.size();
  std::vector<std::vector<int>> asaplookup(N, std::vector<int>(N, INT_MAX));
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

      for (int j = 0; j < N; j++) {
        if ((i == j) || graph[v][j] == INT_MAX) {
          continue;
        }
        if (asaplookup[i][j] > asaplookup[i][v] + graph[v][j]) {
          asaplookup[i][j] = asaplookup[i][v] + graph[v][j];
          que.push({-1 * asaplookup[i][j], j});
        }
      }
    }
  }

  return asaplookup;
}
