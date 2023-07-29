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

inline std::vector<size_t>
orderofj(int i, std::string j_order_type, int sumcost,
         std::vector<Partition> &partitions,
         flat_hash_set<size_t> &checked_partitions, size_t hash_val,
         Logger &logger, int &best_sumcard, int &best_sumcost,
         std::string hf_type, int k, int el, bool complete_search,
         bool &valid_already_found, bool use_upperbound_cost) {
  std::vector<size_t> j_order;

  int partitions_num = partitions.size();

  for (int j = i + 1; j < partitions_num; j++) {

    if (!is_prunable(partitions[i], partitions[j], sumcost, checked_partitions,
                     hash_val, logger, best_sumcard, best_sumcost, hf_type, k,
                     el, complete_search, valid_already_found,
                     use_upperbound_cost)) {
      j_order.push_back(j);
    }
  }

  std::vector<int> dist_from_i;
  if (j_order_type == "nearest") {
    dist_from_i.reserve(j_order.size());
    for (size_t j : j_order) {
      dist_from_i.push_back(partitions[i].dist(partitions[j]));
    }

    std::vector<size_t> sorted_idx = argsort(dist_from_i);
    std::vector<size_t> j_order_sorted;
    for (size_t s : sorted_idx) {
      j_order_sorted.push_back(j_order[s]);
    }
    j_order = j_order_sorted;
  } else if (j_order_type == "pathmatch") {
    dist_from_i.reserve(j_order.size());
    for (size_t j : j_order) {
      dist_from_i.push_back(partitions[i].pathmatch(partitions[j]));
    }

    std::vector<size_t> sorted_idx = argsort(dist_from_i);
    std::vector<size_t> j_order_sorted;
    for (size_t s : sorted_idx) {
      j_order_sorted.push_back(j_order[s]);
    }
    j_order = j_order_sorted;
  }

  return j_order;
}

inline bool merge_df_bb_search(
    std::string j_order_type, std::vector<Partition> &best_partitions,
    std::vector<Partition> &partitions,
    flat_hash_set<size_t> &checked_partitions, Logger &logger,
    int &best_sumcard, int &best_sumcost, std::string hf_type, int k, int el,
    bool complete_search, bool &valid_already_found, bool use_upperbound_cost,
    flat_hash_map<int, int> &base_dist_map) {

  /*
  for (const Partition &p : partitions) {
    for (int i : p.elements) {
      std::cout << i << " ";
    }
    std::cout << "|";
  }
  std::cout << std::endl;
    */

  size_t hash_val = hash_values_of_partitions(partitions);
  checked_partitions.insert(hash_val);
  logger.cum_count++;
  if (logger.print()) {
    return true;
  }

  bool valid_paritions = true;
  int sumcost = 0;
  int satisfying_sumcard = 0;
  int satisfying_sumcost = 0;
  float apc = 0;
  for (Partition p : partitions) {
    sumcost += p.elements.size() * p.cost_of_cover_path;
    if (p.is_satisfying) {
      satisfying_sumcard += p.elements.size();
      satisfying_sumcost += p.elements.size() * p.cost_of_cover_path;
      if (!base_dist_map.empty()) {
        for (int e : p.elements) {
          apc += ((float)p.cost_of_cover_path - (float)base_dist_map[e]) /
                 (float)base_dist_map[e];
        }
      }
    } else {
      valid_paritions = false;
    }
  }

  if (satisfying_sumcard > best_sumcard ||
      (satisfying_sumcard == best_sumcard &&
       satisfying_sumcost < best_sumcost)) {
    best_sumcard = satisfying_sumcard;
    best_sumcost = satisfying_sumcost;
    best_partitions = partitions;

    logger.sum_card = best_sumcard;
    logger.avg_path_cost = apc / (float)best_sumcard;
  }

  if (valid_paritions) {
    if (!valid_already_found) {
      logger.num_evaluated_partitions_till_first_solution = logger.cum_count;
      logger.num_expanded_node_till_first_solution =
          logger.total_num_expanded_node;
      valid_already_found = true;
    }
    logger.valid_count++;
    return !complete_search;
  }

  if (partitions.size() == 1 ||
      (valid_already_found && best_sumcost <= sumcost)) {
    return false;
  }

  int partitions_num = partitions.size();

  for (int i = 0; i < partitions_num; i++) {
    if (partitions[i].is_satisfying) {
      return false;
    }

    std::vector<size_t> j_order =
        orderofj(i, j_order_type, sumcost, partitions, checked_partitions,
                 hash_val, logger, best_sumcard, best_sumcost, hf_type, k, el,
                 complete_search, valid_already_found, use_upperbound_cost);

    for (int j : j_order) {
      Partition partition_i_j = partitions[i].merge(partitions[j], MAX_DIST);

      logger.total_num_expanded_node += partition_i_j.num_expanded_nodes;

      if (partition_i_j.cover_path.size() == 0) {
        logger.skipped_count++;
        continue;
      }

      std::vector<Partition> next_partitions;
      if (!partition_i_j.is_satisfying) {
        next_partitions.push_back(partition_i_j);
      }
      for (int w = 0; w < partitions_num; w++) {
        if (w != i && w != j) {
          next_partitions.push_back(partitions[w]);
        }
      }
      if (partition_i_j.is_satisfying) {
        next_partitions.push_back(partition_i_j);
      }

      bool flag = merge_df_bb_search(
          j_order_type, best_partitions, next_partitions, checked_partitions,
          logger, best_sumcard, best_sumcost, hf_type, k, el, complete_search,
          valid_already_found, use_upperbound_cost, base_dist_map);
      if (flag) {
        return true;
      }
    }
  }

  return false;
}

inline std::vector<Partition>
merge_df_bb(int k, int el, std::string hf_type, std::string j_order_type,
            int source, int goal, HeuristicFuncBase *hfunc, VisibilityFunc *vf,
            std::vector<std::vector<int>> *graph,
            std::vector<std::vector<int>> *asaplookup, bool complete_search,
            bool use_upperbound_cost, Logger &logger,
            flat_hash_map<int, int> &base_dist_map) {
  int N = graph->size();
  std::vector<Partition> best_partitions(0);
  std::vector<Partition> partitions;

  std::vector<int> visible_points_of_i;
  for (int i = 0; i < N; i++) {
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
      partitions.push_back(Partition(k, el, source, goal, hfunc, vf, graph,
                                     asaplookup, tmp_elements, MAX_DIST));
      partitions[partitions.size() - 1].calculate_singleton_h_value();
      logger.total_num_expanded_node +=
          partitions[partitions.size() - 1].num_expanded_nodes;
    }
  }

  logger.tot_node_num = graph->size();
  logger.log_file << graph->size() - 2 - partitions.size()
                  << " Nodes Removed\n";

  int best_sumcard = 0;
  int best_sumcost = MAX_DIST;
  bool valid_found = false;
  flat_hash_set<size_t> checked_partitions;
  merge_df_bb_search(j_order_type, best_partitions, partitions,
                     checked_partitions, logger, best_sumcard, best_sumcost,
                     hf_type, k, el, complete_search, valid_found,
                     use_upperbound_cost, base_dist_map);
  logger.summary();
  return best_partitions;
}
