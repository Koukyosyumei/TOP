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
                           std::vector<int> &target_elements) {
#ifdef _AF
  // int numunseen = 1;
  int numunseen = 0;
  Unseen unseen;
  /*for (int i =0; i < N; i++) {
    unseen[i] = 0;
    }*/
  for (int i : target_elements) {
    unseen[i] = 1;
    numunseen++;
  }

  std::vector<int> visible_points = vf->get_all_visible_points(location_id);
  for (int i : visible_points) {
    if (unseen[i] == 1) {
      unseen[i] = 0;
      numunseen--;
    }
  }
  Node node = Node(location_id, unseen, numunseen, -1, 0, 0);

  if (1) { // unit test: check that unseen and numunseen are correctly sync'd
           // (i.e., check that bitmap implementation and flat_hash_set
           // impelementation would result in same number of unseen points)
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
    // std::cout << " unseen.size = " << unseen.size() << std::endl;
    // std::cout << " numunseen = " << numunseen << std::endl;
    assert(unseen.size() == numunseen);
  }

#else
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
#endif

  node.hash_value = hash_node(node);
  return node;
}

#ifndef _AF
inline
#endif
    std::vector<Node>
    make_children_nodes(VisibilityFunc *vf, Node node, int parent_id) {
  std::vector<Node> children;
#ifndef _AF
  std::vector<int> visible_points;
#endif
  for (const std::pair<int, int> e : vf->graph->at(node.location)) {
    int i = e.first;
    int c = e.second;
    if (i != node.location && c != MAX_DIST) {
#ifdef _AF
      Node child = Node(i, node.unseen, node.numunseen, parent_id, node.g + c,
                        node.hash_value);
#else
      Node child = Node(i, node.unseen, parent_id, node.g + c, node.hash_value);
#endif
#ifdef _HASH2
      child.hash_value ^= hashint(CURLOC_PREFIX * node.location);
      child.hash_value ^= hashint(CURLOC_PREFIX * child.location);
#else
      child.hash_value ^= std::hash<std::string>{}(
          CURLOC_PREFIX + std::to_string(node.location));
      child.hash_value ^= std::hash<std::string>{}(
          CURLOC_PREFIX + std::to_string(child.location));
#endif
#ifdef _AF
      std::vector<int> &visible_points = vf->avp;
      int num_visible_points =
          vf->get_all_visible_points2(node.location, visible_points);
      for (int i = 0; i < num_visible_points; i++) {
        int p = visible_points[i];
        if (child.unseen[p] == 1) {
          child.numunseen--;
          child.unseen[p] = 0;
#ifdef _HASH2
          child.hash_value ^= hashint(p);
#else
          child.hash_value ^= std::hash<std::string>{}(std::to_string(p));
#endif
        }
      }
#else
      visible_points = vf->get_all_visible_points(node.location);
      for (int p : visible_points) {
        if (child.unseen.find(p) != child.unseen.end()) {
          child.unseen.erase(p);
          child.hash_value ^= std::hash<std::string>{}(std::to_string(p));
        }
      }
#endif
      children.push_back(child);
    }
  }

  return children;
}

#ifndef _AF
inline
#endif
    std::pair<int, std::vector<Node>>
    search(HeuristicFuncBase *hfunc, VisibilityFunc *vf, int start_loc,
           int goal_loc, std::vector<int> &target_elements, int upperbound_cost,
           bool use_cache = true) {
  std::priority_queue<std::tuple<int, int, int>> queue;
  Node start = make_root_node(vf, start_loc, target_elements);
  std::vector<Node> nodes = {start};
#ifdef _AF
  int h = hfunc->calculate_hval(nodes[nodes.size() - 1], target_elements);
#else
  int h = hfunc->calculate_hval(nodes[nodes.size() - 1]);
#endif
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
  int front_f = 0;
  while (!queue.empty()) {
    std::cout << 3 << std::endl;  
    iterations++;

    front_status = queue.top();
    front_f = -1 * std::get<0>(front_status);
    node_idx = std::get<2>(front_status);
    queue.pop();

    std::cout << 4 << std::endl;

    if ((!use_cache) ||
        (state_cost[nodes[node_idx].hash_value] == nodes[node_idx].g)) {
      expansions++;
      std::cout << 5 << std::endl;
      if (nodes[node_idx].location == goal_loc &&
#ifdef _AF
          nodes[node_idx].numunseen == 0) {
#else
          nodes[node_idx].unseen.size() == 0) {
#endif
        return std::make_pair(expansions, extract_solution(node_idx, nodes));
      }
      std::cout << 6 << std::endl;

      // std::cout << front_h << " " << upperbound_cost << std::endl;
      // if (front_f > upperbound_cost) {
      //  return {expansions, {}};
      // }

      children = make_children_nodes(vf, nodes[node_idx], node_idx);
      std::cout << 7 << std::endl;
      for (Node child : children) {
        std::cout << 8 << std::endl;  
        nodes.emplace_back(child);
#ifdef _AF
        h = hfunc->calculate_hval(nodes[nodes.size() - 1], target_elements);
#else
        h = hfunc->calculate_hval(nodes[nodes.size() - 1]);
#endif
        old_succ_g = MAX_DIST;
        succ_g = nodes[nodes.size() - 1].g;
        if (h + succ_g <= upperbound_cost) {
          if (use_cache) {
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
          else {  
          std::cout << 1 << std::endl;  
          queue.push({-1 * (h + succ_g), -1 * h, nodes.size() - 1});
          std::cout << 2 << std::endl;
        }

        }       }
      std::cout << 9 << std::endl;
    } else {
      skipped++;
    }
  }

  std::vector<Node> emp = {};
  return std::make_pair(expansions, emp);
}
