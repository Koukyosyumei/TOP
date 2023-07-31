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

#ifdef _AF
#include <bitset>
using Unseen = std::bitset<MAXPARTSIZE>;
#endif
using phmap::flat_hash_set;

const int MAX_DIST = 1e5; // std::numeric_limits<int>::max();
#ifdef _HASH2
std::hash<int> hashint2;
inline size_t hashint(int x) {
  return hashint2(x + 0x9e3779b9) + x << 6 + x >> 2; // XXX??? Icargo cult hash... I don't really understand this..
}
  
const int CURLOC_PREFIX = 293; // some prime
#else
const std::string CURLOC_PREFIX = "cur_loc:";
#endif



struct Node {
  int location;
#ifdef _AF
  Unseen unseen; 
  int numunseen;
#else
  flat_hash_set<int> unseen;
#endif
  int parent_id = -1;
  int g = 0;
  size_t hash_value;

  Node(){};
#ifdef _AF
  Node(int location, Unseen unseen, int numunseen, int parent_id, int g,
       size_t hash_value)
      : location(location), unseen(unseen), numunseen(numunseen), parent_id(parent_id), g(g),
        hash_value(hash_value) {}
#else
  Node(int location, flat_hash_set<int> unseen, int parent_id, int g,
       size_t hash_value)
      : location(location), unseen(unseen), parent_id(parent_id), g(g),
        hash_value(hash_value) {}
#endif

};

#ifndef _AF
inline
#endif
size_t hash_node(const Node &node) {
  size_t seed =
#ifdef _HASH2
    hashint(CURLOC_PREFIX*node.location);
#else
    std::hash<std::string>{}(CURLOC_PREFIX + std::to_string(node.location));
#endif
#ifdef _AF
  for (int x = 0; x < N; x++) {
    if (node.unseen[x] == 1) {
#ifdef _HASH2
      seed ^= hashint(x);
#else
      seed ^= std::hash<std::string>{}(std::to_string(x));
#endif
    }
  }
#else
  for (int x : node.unseen) {
    seed ^= std::hash<std::string>{}(std::to_string(x));
  }
#endif
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
