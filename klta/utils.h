#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "parallel_hashmap/phmap.h"

using phmap::flat_hash_set;

const int MAX_DIST = 1e5; // std::numeric_limits<int>::max();
const std::string CURLOC_PREFIX = "cur_loc:";

struct Node {
  int location;
  flat_hash_set<int> unseen;
  int parent_id = -1;
  int g = 0;
  size_t hash_value;

  Node(){};
  Node(int location, flat_hash_set<int> unseen, int parent_id, int g,
       size_t hash_value)
      : location(location), unseen(unseen), parent_id(parent_id), g(g),
        hash_value(hash_value) {}
};

inline size_t hash_node(const Node &node) {
  size_t seed =
      std::hash<std::string>{}(CURLOC_PREFIX + std::to_string(node.location));
  for (int x : node.unseen) {
    seed ^= std::hash<std::string>{}(std::to_string(x));
  }
  return seed;
}

inline std::vector<Node> extract_solution(int this_id,
                                          std::vector<Node> &nodes) {
  int node_id = this_id;
  std::vector<Node> solution;
  while (nodes[node_id].parent_id != -1) {
    solution.push_back(nodes[node_id]);
    node_id = nodes[node_id].parent_id;
  }
  solution.push_back(nodes[node_id]);
  std::reverse(solution.begin(), solution.end());
  return solution;
}
