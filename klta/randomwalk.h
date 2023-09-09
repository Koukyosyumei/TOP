#pragma once
#include <random>
#include <vector>

#include "covering_search.h"
#include "heuristic.h"
#include "utils.h"
#include "visibility.h"

inline std::vector<float>
randomwalker(HeuristicFuncBase *hf, VisibilityFunc *vf, int start_loc,
             int goal_loc, std::vector<int> &target_elements, float m_ratio,
             flat_hash_map<int, int> &base_dist_map, int seed = 42) {
  std::vector<float> costs;
  for (int transit_loc : target_elements) {

    std::mt19937 rng(seed);
    int m = (float)base_dist_map[transit_loc] * m_ratio;

    float first_cost;
    std::vector<int> first_path;
    first_path.push_back(start_loc);
    for (int i = 0; i < m - 1; i++) {
      std::uniform_int_distribution<int> dist(
          0, vf->graph->at(first_path[i]).size() - 1);
      int randomIndex = dist(rng);
      first_path.push_back(vf->graph->at(first_path[i])[randomIndex].first);
      first_cost += (float)vf->graph->at(first_path[i])[randomIndex].second;
    }

    std::vector<int> tmp_target_elements = {transit_loc};
    std::pair<int, std::vector<Node>> latter_path =
        search(hf, vf, first_path[first_path.size() - 1], goal_loc,
               tmp_target_elements, MAX_DIST);
    costs.push_back(first_cost +
                    (float)latter_path.second[latter_path.second.size() - 1].g);
  }

  return costs;
}
