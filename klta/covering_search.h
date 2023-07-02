#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "heuristic.h"
#include "parallel_hashmap/phmap.h"
#include "utils.h"
#include "visibility.h"

using phmap::flat_hash_map;

inline Node make_root_node(VisibilityFunc *vf, int location_id,
                           std::vector<int> target_elements) {
  flat_hash_set<int> unseen;
  for (int i : target_elements) {
    unseen.insert(i);
  }

  std::vector<int> visible_points = vf->get_all_visible_points(location_id);
  for (int i : visible_points) {
    if (unseen.find(i) != unseen.end()) {
      unseen.erase(i);
    }
  }

  Node node = Node(location_id, unseen, -1, 0, 0);
  node.hash_value = hash_node(node);
  return node;
}

inline std::vector<Node> make_children_nodes(VisibilityFunc *vf, Node node,
                                             int parent_id) {
  std::vector<Node> children;
  std::vector<int> visible_points;

  for (int i = 0; i < vf->graph.size(); i++) {
    if (i != node.location && vf->graph[node.location][i] != MAX_DIST) {
      Node child = Node(i, node.unseen, parent_id,
                        node.g + vf->graph[node.location][i], node.hash_value);
      child.hash_value ^= std::hash<std::string>{}(
          CURLOC_PREFIX + std::to_string(node.location));
      child.hash_value ^= std::hash<std::string>{}(
          CURLOC_PREFIX + std::to_string(child.location));
      visible_points = vf->get_all_visible_points(node.location);
      for (int p : visible_points) {
        if (child.unseen.find(p) != child.unseen.end()) {
          child.unseen.erase(p);
          child.hash_value ^= std::hash<std::string>{}(std::to_string(p));
        }
      }
      children.push_back(child);
    }
  }

  return children;
}

inline std::pair<int, std::vector<Node>>
search(HeuristicFuncBase *hfunc, VisibilityFunc *vf, int start_loc,
       int goal_loc, std::vector<int> &target_elements, int upperbound_cost) {
  std::priority_queue<std::tuple<int, int, int>> queue;
  Node start = make_root_node(vf, start_loc, target_elements);
  std::vector<Node> nodes = {start};
  int h = hfunc->calculate_hval(nodes[nodes.size() - 1]);
  // if (h <= upperbound_cost) {
  queue.push(std::make_tuple(-1 * (h + nodes[nodes.size() - 1].g), -1 * h,
                             nodes.size() - 1));
  // }
  flat_hash_map<size_t, int> state_cost = {{nodes[0].hash_value, 0}};
  std::tuple<int, int, int> front_status;
  std::vector<Node> children;
  int node_idx, succ_g, old_succ_g;
  int iterations = 0;
  int expansions = 0;
  int skipped = 0;
  while (!queue.empty()) {
    iterations++;

    front_status = queue.top();
    node_idx = std::get<2>(front_status);
    queue.pop();

    if (state_cost[nodes[node_idx].hash_value] == nodes[node_idx].g) {
      expansions++;
      if (nodes[node_idx].location == goal_loc &&
          nodes[node_idx].unseen.size() == 0) {
        return std::make_pair(expansions, extract_solution(node_idx, nodes));
      }

      children = make_children_nodes(vf, nodes[node_idx], node_idx);
      for (Node child : children) {
        nodes.emplace_back(child);
        h = hfunc->calculate_hval(nodes[nodes.size() - 1]);
        old_succ_g = MAX_DIST;
        succ_g = nodes[nodes.size() - 1].g;
        if (h + succ_g <= upperbound_cost) {
          if (state_cost.find(child.hash_value) != state_cost.end()) {
            old_succ_g = state_cost[child.hash_value];
            if (succ_g < old_succ_g) {
              queue.push({-1 * (h + succ_g), -1 * h, nodes.size() - 1});
              state_cost[child.hash_value] = succ_g;
            }
          } else {
            queue.push({-1 * (h + succ_g), -1 * h, nodes.size() - 1});
            state_cost.emplace(child.hash_value, succ_g);
          }
        }
      }
    } else {
      skipped++;
    }
  }

  std::vector<Node> emp = {};
  return std::make_pair(expansions, emp);
}
