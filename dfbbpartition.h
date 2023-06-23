#pragma once
#include "covering_search.h"
#include "partition.h"
#include "utils.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>

inline std::vector<size_t>
orderofj(int i, std::string j_order_type, int sumcost,
         std::vector<Partition> &partitions,
         std::unordered_set<size_t> &checked_partitions, size_t hash_val,
         Logger &logger, int &best_sumcard, int &best_sumcost, int k, int el,
         bool complete_search, int log_count, bool &valid_already_found,
         bool use_upperbound_cost) {
  std::vector<size_t> j_order;

  int partitions_num = partitions.size();

  for (int j = i + 1; j < partitions_num; j++) {

    if (!is_prunable(partitions[i], partitions[j], sumcost, checked_partitions,
                     hash_val, logger, best_sumcard, best_sumcost, k, el,
                     complete_search, log_count, valid_already_found,
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

    // std::sort(j_order.begin(), j_order.end(), [&](size_t a, size_t b) {
    //  return dist_from_i[a] < dist_from_i[b];
    // });
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
    std::unordered_set<size_t> &checked_partitions, Logger &logger,
    int &best_sumcard, int &best_sumcost, int k, int el, bool complete_search,
    int log_count, bool &valid_already_found, bool use_upperbound_cost) {
  size_t hash_val = hash_values_of_partitions(partitions);
  checked_partitions.insert(hash_val);
  logger.cum_count++;

  if (logger.cum_count != 0 && logger.cum_count % log_count == 0) {
    logger.print();
  }

  bool valid_paritions = true;
  int sumcost = 0;
  int satisfying_sumcard = 0;
  int satisfying_sumcost = 0;
  for (Partition p : partitions) {
    sumcost += p.elements.size() * p.cost_of_cover_path;
    if (p.is_satisfying) {
      satisfying_sumcard += p.elements.size();
      satisfying_sumcost += p.elements.size() * p.cost_of_cover_path;
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

    std::cout << "Best Cardinarity: " << best_sumcard << ", Best AVG Cost: "
              << (float)best_sumcost / (float)best_sumcard << "\n";
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

    std::vector<size_t> j_order = orderofj(
        i, j_order_type, sumcost, partitions, checked_partitions, hash_val,
        logger, best_sumcard, best_sumcost, k, el, complete_search, log_count,
        valid_already_found, use_upperbound_cost);

    for (int j : j_order) {
      int upperbound_cost = INT_MAX;
      if (valid_already_found && use_upperbound_cost) {
        upperbound_cost =
            best_sumcost - (sumcost - partitions[i].cost_of_cover_path -
                            partitions[j].cost_of_cover_path);
        upperbound_cost /=
            (partitions[i].elements.size() + partitions[j].elements.size());
      }
      Partition partition_i_j =
          partitions[i].merge(partitions[j], upperbound_cost);

      logger.total_num_expanded_node += partition_i_j.num_expanded_nodes;

      // if (!partition_i_j.is_satisfy_el()) {
      //   logger.skipped_count++;
      //  continue;
      // }

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
          logger, best_sumcard, best_sumcost, k, el, complete_search, log_count,
          valid_already_found, use_upperbound_cost);
      if (flag) {
        return true;
      }
    }
  }

  return false;
}

inline std::vector<Partition>
merge_df_bb(int k, int el, std::string j_order_type, int source, int goal,
            HeuristicFuncBase *hfunc, VisibilityFunc *vf,
            std::vector<std::vector<int>> *graph,
            std::vector<std::vector<int>> *asaplookup, bool complete_search,
            int verbose, bool use_upperbound_cost) {
  int N = graph->size();
  std::vector<Partition> best_partitions(0);
  std::vector<Partition> partitions;
  Logger logger;

  std::vector<int> visible_points_of_i;
  for (int i = 0; i < N; i++) {
    if (i == source || i == goal) {
      continue;
    }
    bool is_valid = false;
    visible_points_of_i = vf->get_all_watchers(i);
    for (int j : visible_points_of_i) {
      if ((asaplookup->at(source)[j] != INT_MAX) &&
          (asaplookup->at(j)[goal] != INT_MAX)) {
        is_valid = true;
        break;
      }
    }
    if (is_valid) {
      std::vector<int> tmp_elements = {i};
      partitions.push_back(Partition(k, el, source, goal, hfunc, vf, graph,
                                     asaplookup, tmp_elements, INT_MAX));
      partitions[partitions.size() - 1].calculate_singleton_h_value();
      logger.total_num_expanded_node +=
          partitions[partitions.size() - 1].num_expanded_nodes;
    }
  }

  std::cout << graph->size() - 2 - partitions.size() << " Nodes Removed\n";

  int best_sumcard = 0;
  int best_sumcost = INT_MAX;
  bool valid_found = false;
  std::unordered_set<size_t> checked_partitions;
  merge_df_bb_search(j_order_type, best_partitions, partitions,
                     checked_partitions, logger, best_sumcard, best_sumcost, k,
                     el, complete_search, verbose, valid_found,
                     use_upperbound_cost);
  logger.summary();
  std::cout << "- Number of Anonymized Paths: " << best_sumcard << "\n";
  std::cout << "- Average Cost of Anonymized Paths: "
            << (float)best_sumcost / (float)best_sumcard << "\n";
  return best_partitions;
}
