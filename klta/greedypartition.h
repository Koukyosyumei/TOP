#pragma once
#include "covering_search.h"
#include "partition.h"
#include "utils.h"
#include <random>
#include <vector>

#ifndef _AF
inline
#endif
    bool
    greedypartition_search(std::string j_order_type,
                           std::vector<Partition> &best_partitions,
                           std::vector<Partition *> subsets,
                           std::vector<Partition *> unassigned,
                           flat_hash_set<size_t> &checked_partitions,
                           Logger &logger, int &best_nap, float &best_mac,
                           std::string hf_type, int k, int el,
                           bool complete_search, bool &valid_already_found,
                           bool use_upperbound_cost, bool use_prune,
                           flat_hash_map<int, int> &base_dist_map) {

  size_t hash_val = hash_values_of_partitions(subsets);
  checked_partitions.insert(hash_val);
  if (logger.print()) {
    return true;
  }

  bool valid_paritions = true;
  int satisfying_nap = 0;
  float sum_ac = 0;
  float satisfying_mac = 0;
  for (Partition *p : subsets) {
    if (p->is_satisfying) {
      satisfying_nap += p->elements.size();
      satisfying_mac += p->ac;
    } else {
      valid_paritions = false;
    }
    sum_ac += p->ac;
  }
  satisfying_mac = satisfying_mac / (float)(satisfying_nap);

  if (unassigned.size() == 0) {
    logger.cum_count++;

    if (satisfying_nap > best_nap ||
        (satisfying_nap == best_nap && satisfying_mac < best_mac)) {
      best_nap = satisfying_nap;
      best_mac = satisfying_mac;
      best_partitions.clear();
      best_partitions.reserve(subsets.size());
      for (Partition *p : subsets) {
        best_partitions.push_back(*p);
      }

      logger.sum_card = best_nap;
      logger.avg_path_cost = best_mac;
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

  } else {
    int n = 0;
    Partition *picked = unassigned[n];
    unassigned.erase(unassigned.begin() + n);

    subsets.push_back(picked);
    bool flag = greedypartition_search(
        j_order_type, best_partitions, subsets, unassigned, checked_partitions,
        logger, best_nap, best_mac, hf_type, k, el, complete_search,
        valid_already_found, use_upperbound_cost, use_prune, base_dist_map);

    if (flag) {
      return true;
    }

    subsets.resize(subsets.size() - 1);

    std::vector<int> idxs(subsets.size());
    std::iota(idxs.begin(), idxs.end(), 0);
    // std::mt19937 engine(logger.cum_count);
    // std::shuffle(idxs.begin(), idxs.end(), engine);

    for (int i : idxs) {
      Partition *p_tmp = subsets[i];

      if (use_prune &&
          is_prunable(subsets[i], picked, sum_ac, checked_partitions, hash_val,
                      logger, best_nap, best_mac, hf_type, k, el,
                      complete_search, valid_already_found, use_upperbound_cost,
                      false)) {
        continue;
      }

      // new_idxs.push_back(i);
      subsets[i] = subsets[i]->merge(picked, MAX_DIST);
      logger.total_num_expanded_node += subsets[i]->num_expanded_nodes;
      subsets[i]->cover_path.clear();
      subsets[i]->cover_path.shrink_to_fit();
      // if (use_prune && subsets[i].cover_path.size() == 0) {
      //  logger.skipped_count++;
      //  continue;
      //}

      bool flag = greedypartition_search(
          j_order_type, best_partitions, subsets, unassigned,
          checked_partitions, logger, best_nap, best_mac, hf_type, k, el,
          complete_search, valid_already_found, use_upperbound_cost, use_prune,
          base_dist_map);
      if (flag) {
        return true;
      }
      subsets[i] = p_tmp;
    }
  }

  return false;
}

#ifndef _AF
inline
#endif
    std::vector<Partition>
    greedypartition(int k, int el, std::string hf_type,
                    std::string j_order_type, int source, int goal,
                    HeuristicFuncBase *hfunc, VisibilityFunc *vf,
                    std::vector<std::vector<std::pair<int, int>>> *graph,
                    std::vector<std::vector<int>> *asaplookup,
                    std::vector<int> &transit_candidates, bool complete_search,
                    bool use_upperbound_cost, Logger &logger, bool use_prune,
                    flat_hash_map<int, int> &base_dist_map) {
  int N = graph->size();
  std::vector<Partition> best_partitions(0);
  std::vector<Partition *> subsets;
  std::vector<Partition *> unassigned;

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
      unassigned.push_back(new Partition(k, el, source, goal, hfunc, vf, graph,
                                         asaplookup, &base_dist_map,
                                         tmp_elements, MAX_DIST));
      unassigned[unassigned.size() - 1]->calculate_singleton_h_value();
      logger.total_num_expanded_node +=
          unassigned[unassigned.size() - 1]->num_expanded_nodes;
      unassigned[unassigned.size() - 1]->cover_path.clear();
      unassigned[unassigned.size() - 1]->cover_path.shrink_to_fit();
    }
  }

  logger.tot_node_num = transit_candidates.size();
  logger.log_file << transit_candidates.size() - unassigned.size()
                  << " Nodes Removed\n";

  int best_nap = 0;
  float best_mac = (float)MAX_DIST;
  bool valid_found = false;
  flat_hash_set<size_t> checked_partitions;
  greedypartition_search(j_order_type, best_partitions, subsets, unassigned,
                         checked_partitions, logger, best_nap, best_mac,
                         hf_type, k, el, complete_search, valid_found,
                         use_upperbound_cost, use_prune, base_dist_map);
  logger.summary();
  return best_partitions;
}
