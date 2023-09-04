#pragma once
#include <random>
#include <vector>

#include "asap.h"
#include "covering_search.h"
#include "heuristic.h"
#include "utils.h"
#include "visibility.h"

inline size_t
hash_value_assignments(std::vector<std::vector<int>> &assignments) {
  size_t result = 0;
  for (std::vector<int> &elements : assignments) {
    size_t val = 0;
    for (int e : elements) {
      val ^= std::hash<std::string>{}(std::to_string(e));
    }
    result ^= std::hash<std::string>{}(std::to_string(val));
  }
  return result;
}

inline int get_center(int N, std::vector<std::vector<int>> *asaplookup,
                      std::vector<int> &elements) {
  int min_dist = MAX_DIST;
  int min_id = -1;
  for (int n = 0; n < N; n++) {
    int tmp_dist = 0;
    for (int k : elements) {
      // if (n == k) {
      //   tmp_dist = MAX_DIST;
      //   break;
      // }
      tmp_dist += asaplookup->at(n)[k];
    }
    if (min_dist > tmp_dist) {
      min_dist = tmp_dist;
      min_id = n;
    }
  }
  return min_id;
}

// assume that all edge costs are one
inline std::vector<float>
clustering_randomwalker(HeuristicFuncBase *hf, VisibilityFunc *vf,
                        int start_loc, int goal_loc,
                        std::vector<int> &target_elements, int m,
                        std::vector<std::vector<int>> &assignments,
                        std::vector<int> &center, int seed = 42) {
  std::mt19937 rng(seed);
  std::vector<float> costs;

  for (int i = 0; i < assignments.size(); i++) {
    if (assignments[i].size() == 0) {
      continue;
    }
    float first_cost = vf->asaplookup->at(start_loc)[center[i]];
    std::vector<int> first_path = {start_loc};
    if (m > first_cost) {
      first_cost = 0;
      while (first_cost + vf->asaplookup->at(
                              first_path[first_path.size() - 1])[center[i]] <
             m) {
        std::uniform_int_distribution<int> dist(
            0, vf->graph->at(first_path[first_path.size() - 1]).size() - 1);
        int randomIndex = dist(rng);
        std::pair<int, int> e =
            vf->graph->at(first_path[first_path.size() - 1])[randomIndex];
        first_path.push_back(e.first);
        first_cost += e.second;
      }
      first_cost +=
          vf->asaplookup->at(first_path[first_path.size() - 1])[center[i]];
    }
    for (int transit_loc : assignments[i]) {
      std::vector<int> tmp_target_elements = {transit_loc};
      std::pair<int, std::vector<Node>> latter_path =
          search(hf, vf, center[i], goal_loc, tmp_target_elements, MAX_DIST);
      costs.push_back(
          first_cost +
          (float)latter_path.second[latter_path.second.size() - 1].g);
    }
  }
  return costs;
}

inline std::pair<std::vector<std::vector<int>>, std::vector<int>>
clustering(int k, std::vector<int> &transit_candidates,
           std::vector<std::vector<int>> *asaplookup, int max_itr = 100) {
  std::vector<std::vector<int>> assignments;
  std::vector<int> center;
  int N = asaplookup->size();

  // initial assignments
  int num_partition = transit_candidates.size() / k;
  for (int j = 0; j < num_partition - 1; j++) {
    std::vector<int> elements;
    for (int i = j * k; i < (j + 1) * k; i++) {
      elements.emplace_back(transit_candidates[i]);
    }
    assignments.emplace_back(elements);
  }
  std::vector<int> elements;
  for (int i = k * (num_partition - 1); i < transit_candidates.size(); i++) {
    elements.emplace_back(transit_candidates[i]);
  }
  assignments.emplace_back(elements);
  size_t cur_hash_val = hash_value_assignments(assignments);

  for (int i = 0; i < max_itr; i++) {

    // calculate the new center
    center.clear();
    for (int j = 0; j < assignments.size(); j++) {
      center.push_back(get_center(N, asaplookup, assignments[j]));
      assignments[j].clear();
    }

    // update the assignment
    for (int t : transit_candidates) {
      int min_dist = MAX_DIST;
      int min_k;
      for (int k = 0; k < center.size(); k++) {
        if (min_dist > asaplookup->at(center[k])[t]) {
          min_k = k;
          min_dist = asaplookup->at(center[k])[t];
        }
      }
      assignments[min_k].emplace_back(t);
    }

    // chekc the difference
    size_t new_hash_val = hash_value_assignments(assignments);
    if (cur_hash_val == new_hash_val) {
      break;
    }
    cur_hash_val = new_hash_val;
  }

  bool cont_flag = true;
  while (cont_flag) {
    cont_flag = false;
    for (int i = 0; i < assignments.size(); i++) {
      if (assignments[i].size() > 0 && assignments[i].size() < k) {
        cont_flag = true;
        int min_dist = MAX_DIST;
        int min_c;
        for (int j = 0; j < assignments.size(); j++) {
          if (i != j) {
            int tmp_dist = asaplookup->at(center[i])[center[j]];
            if (min_dist > tmp_dist) {
              min_dist = tmp_dist;
              min_c = j;
            }
          }
        }
        for (int e : assignments[i]) {
          assignments[min_c].push_back(e);
        }
        center[min_c] = get_center(N, asaplookup, assignments[min_c]);
        center[i] = -1;
        assignments[i].clear();
      }
    }
  }

  return make_pair(assignments, center);
}
