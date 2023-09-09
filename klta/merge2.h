#pragma once
#include "covering_search.h"
#include "parallel_hashmap/phmap.h"
#include "partition.h"
#include "utils.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <tuple>
#include <vector>

#include <algorithm>
#include <random>

using phmap::flat_hash_map;
using phmap::flat_hash_set;

std::vector<std::tuple<float, size_t, size_t>>
orderofij(std::string ij_order_type, float sum_ac,
          std::vector<Partition *> &partitions,
          flat_hash_set<size_t> &checked_partitions, size_t hash_val,
          Logger &logger, int &best_nap, float &best_mac, std::string hf_type,
          int k, int el, bool complete_search, bool &valid_already_found,
          bool use_upperbound_cost) {
  // std::vector<size_t> j_order;
  std::vector<std::tuple<float, size_t, size_t>> ij_order;

  int partitions_num = partitions.size();

  for (int i = 0; i < partitions_num; i++) {
    if (partitions[i]->is_satisfying) // XXX moved from merge_df_bb_search
      continue;
    for (int j = i + 1; j < partitions_num; j++) {
      if (!is_prunable(partitions[i], partitions[j], sum_ac, checked_partitions,
                       hash_val, logger, best_nap, best_mac, hf_type, k, el,
                       complete_search, valid_already_found,
                       use_upperbound_cost)) {
        float h_ij =
            std::max(partitions[i]->cost_of_cover_path,
                     partitions[j]->cost_of_cover_path) *
            (partitions[i]->elements.size() + partitions[j]->elements.size());
        if (ij_order_type == "ascnear") {
          h_ij = partitions[i]->dist(partitions[j]);
        } else if (ij_order_type == "decnear") {
          h_ij = -1 * partitions[i]->dist(partitions[j]);
        } else if (ij_order_type == "deccost") {
          h_ij *= -1;
        } else if (ij_order_type == "adacost+") {
          if (valid_already_found) {
            h_ij *= -1;
          }
        } else if (ij_order_type == "adacost-") {
          if (!valid_already_found) {
            h_ij *= -1;
          }
        } else if (ij_order_type == "adacostnear+") {
          h_ij += 0.1 * partitions[i]->dist(partitions[j]);
          if (valid_already_found) {
            h_ij *= -1;
          }
        } else if (ij_order_type == "adacostnear-") {
          h_ij += 0.1 * partitions[i]->dist(partitions[j]);
          if (!valid_already_found) {
            h_ij *= -1;
          }
        } else if (ij_order_type == "adanearcost+") {
          h_ij = h_ij * 0.1 + partitions[i]->dist(partitions[j]);
          if (valid_already_found) {
            h_ij *= -1;
          }
        } else if (ij_order_type == "adanearcost-") {
          h_ij = h_ij * 0.1 + partitions[i]->dist(partitions[j]);
          if (!valid_already_found) {
            h_ij *= -1;
          }
        } else if (ij_order_type == "adanear+") {
          h_ij = partitions[i]->dist(partitions[j]);
          if (valid_already_found) {
            h_ij *= -1;
          }
        } else if (ij_order_type == "adanear-") {
          h_ij = partitions[i]->dist(partitions[j]);
          if (!valid_already_found) {
            h_ij *= -1;
          }
        }

        ij_order.push_back(std::tuple<float, size_t, size_t>(h_ij, i, j));
      }
    }
  }
  // sort ij_order   according to ij_order_type
  if (ij_order_type == "random") { // trivial random ordering
    auto rng = std::default_random_engine{};
    std::shuffle(ij_order.begin(), ij_order.end(), rng);
  } else {
    std::sort(ij_order.begin(), ij_order.end());
  }

  return ij_order;
}

bool merge_df_bb_search2(std::string ij_order_type,
                         std::vector<Partition> &best_partitions,
                         std::vector<Partition *> &partitions,
                         flat_hash_set<size_t> &checked_partitions,
                         Logger &logger, int &best_nap, float &best_mac,
                         std::string hf_type, int k, int el, float m,
                         bool complete_search, bool &valid_already_found,
                         bool use_upperbound_cost,
                         flat_hash_map<int, int> &base_dist_map) {

  size_t hash_val = hash_values_of_partitions(partitions);
  checked_partitions.insert(hash_val);
  logger.cum_count++;
  if (logger.print()) {
    return true;
  }

  bool valid_paritions = true;
  int satisfying_nap = 0;
  float sum_ac = 0;
  float satisfying_mac = 0;
  for (Partition *p : partitions) {
    if (p->is_satisfying) {
      satisfying_nap += p->elements.size();
      satisfying_mac += p->ac;
    } else {
      valid_paritions = false;
    }
    sum_ac += p->ac;
  }
  satisfying_mac = satisfying_mac / (float)(satisfying_nap);

  if (satisfying_nap > best_nap ||
      (satisfying_nap == best_nap && satisfying_mac < best_mac)) {
    best_nap = satisfying_nap;
    best_mac = satisfying_mac;
    best_partitions.clear();
    best_partitions.reserve(partitions.size());
    for (Partition *p : partitions) {
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

  if (partitions.size() == 1 ||
      (valid_already_found && best_mac * (float)best_nap <= sum_ac)) {
    return false;
  }

  int partitions_num = partitions.size();

  std::vector<std::tuple<float, size_t, size_t>> ij_order =
      orderofij(ij_order_type, sum_ac, partitions, checked_partitions, hash_val,
                logger, best_nap, best_mac, hf_type, k, el, complete_search,
                valid_already_found, use_upperbound_cost);

  for (std::tuple<float, size_t, size_t> &ij : ij_order) {
    int i = std::get<1>(ij);
    int j = std::get<2>(ij);
    int upperbound_cost = MAX_DIST;
    Partition *partition_i_j =
        partitions[i]->merge(partitions[j], upperbound_cost);

    logger.total_num_expanded_node += partition_i_j->num_expanded_nodes;

    if (partition_i_j->cover_path.size() == 0) {
      logger.skipped_count++;
      continue;
    }

    if (m < 0) {
      partition_i_j->cover_path.clear();
      partition_i_j->cover_path.shrink_to_fit();
    }

    std::vector<Partition *> next_partitions;
    if (!partition_i_j->is_satisfying) {
      next_partitions.emplace_back(partition_i_j);
    }
    for (int w = 0; w < partitions_num; w++) {
      if (w != i && w != j) {
        next_partitions.emplace_back(partitions[w]);
      }
    }
    if (partition_i_j->is_satisfying) {
      next_partitions.emplace_back(partition_i_j);
    }

    bool flag = merge_df_bb_search2(
        ij_order_type, best_partitions, next_partitions, checked_partitions,
        logger, best_nap, best_mac, hf_type, k, el, m, complete_search,
        valid_already_found, use_upperbound_cost, base_dist_map);
    delete partition_i_j;
    if (flag) {
      return true;
    }
  }

  return false;
}
