#pragma once
#include "covering_search.h"
#include "parallel_hashmap/phmap.h"
#include "partition.h"
#include "utils.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <vector>

using phmap::flat_hash_map;
using phmap::flat_hash_set;

inline void trunc_path(int goal, std::vector<Partition> &partitions,
                       float m_ratio, VisibilityFunc *vf,
                       std::vector<std::vector<int>> *asaplookup,
                       flat_hash_map<int, int> &base_dist_map, Logger &logger) {
  float ac = 0;
  float sum_cardinarity = 0;
  for (Partition &p : partitions) {
    if (p.is_satisfying) {
      sum_cardinarity += (float)p.elements.size();
      for (int t : p.elements) {
        int m = (int)(m_ratio * (float)base_dist_map[t]);
        if (p.cover_path.size() > m) {
#ifdef _AF
          if (p.cover_path[m].unseen[t]) {
            ac +=
                ((float)m + (float)asaplookup->at(p.cover_path[m].location)[t] +
                 (float)asaplookup->at(t)[goal] - (float)base_dist_map[t]) /
                ((float)base_dist_map[t]);
          } else {
            ac += ((float)m +
                   (float)asaplookup->at(p.cover_path[m].location)[goal] -
                   (float)base_dist_map[t]) /
                  ((float)base_dist_map[t]);
          }
#else
          if (p.cover_path[m].unseen.find(t) != p.cover_path[m].unseen.end()) {
            ac +=
                ((float)m + (float)asaplookup->at(p.cover_path[m].location)[t] +
                 (float)asaplookup->at(t)[goal] - (float)base_dist_map[t]) /
                ((float)base_dist_map[t]);
          } else {
            ac += ((float)m +
                   (float)asaplookup->at(p.cover_path[m].location)[goal] -
                   (float)base_dist_map[t]) /
                  ((float)base_dist_map[t]);
          }
#endif
        } else {
          ac += ((float)p.cost_of_cover_path - (float)base_dist_map[t]) /
                ((float)base_dist_map[t]);
        }
      }
    }
  }
  logger.avg_path_cost = ac / sum_cardinarity;
}

#ifndef _AF
inline
#endif
    std::vector<Partition>
    merge_df_bb(int k, int el, float m_ratio, std::string hf_type,
                std::string j_order_type, int source, int goal,
                HeuristicFuncBase *hfunc, VisibilityFunc *vf,
                std::vector<std::vector<std::pair<int, int>>> *graph,
                std::vector<std::vector<int>> *asaplookup,
                std::vector<int> &transit_candidates, bool complete_search,
                bool use_upperbound_cost, Logger &logger,
                flat_hash_map<int, int> &base_dist_map) {
  int N = graph->size();
  std::vector<Partition> best_partitions(0);
  std::vector<Partition *> partitions;

  std::vector<int> visible_points_of_i;
  for (int i : transit_candidates) {
    if (i == source || i == goal) {
      continue;
    }
    bool is_valid = false;
    visible_points_of_i = vf->get_all_watchers(i);
    for (int j : visible_points_of_i) {
      if ((asaplookup->at(source)[j] != MAX_DIST) &&
          (asaplookup->at(j)[goal] != MAX_DIST)) {
        is_valid = true;
        break;
      }
    }
    if (is_valid) {
      std::vector<int> tmp_elements = {i};
      partitions.push_back(new Partition(k, el, source, goal, hfunc, vf, graph,
                                         asaplookup, &base_dist_map,
                                         tmp_elements, MAX_DIST));
      partitions[partitions.size() - 1]->calculate_singleton_h_value();
      logger.total_num_expanded_node +=
          partitions[partitions.size() - 1]->num_expanded_nodes;
      partitions[partitions.size() - 1]->cover_path.clear();
      partitions[partitions.size() - 1]->cover_path.shrink_to_fit();
    }
  }

  logger.tot_node_num = transit_candidates.size();
  logger.log_file << transit_candidates.size() - partitions.size()
                  << " Nodes Removed\n";

  int best_sumcard = 0;
  float best_sumcost = (float)MAX_DIST;
  bool valid_found = false;
  flat_hash_set<size_t> checked_partitions;
  merge_df_bb_search2(j_order_type, best_partitions, partitions,
                      checked_partitions, logger, best_sumcard, best_sumcost,
                      hf_type, k, el, m_ratio, complete_search, valid_found,
                      use_upperbound_cost, base_dist_map);
  if (m_ratio > 0) {
    trunc_path(goal, best_partitions, m_ratio, vf, asaplookup, base_dist_map,
               logger);
  }
  logger.summary();
  return best_partitions;
}
