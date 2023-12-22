#pragma once
#include "covering_search.h"
#include "parallel_hashmap/phmap.h"
#include "partition.h"
#include "utils.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using phmap::flat_hash_map;
using phmap::flat_hash_set;

inline std::pair<int, int> selectTwoRandomNumbers(int N, std::mt19937 &gen) {
  std::vector<int> numbers(N);
  for (int i = 0; i < N; ++i) {
    numbers[i] = i;
  }
  std::shuffle(numbers.begin(), numbers.end(), gen);
  return std::make_pair(numbers[0], numbers[1]);
}

#ifndef _AF
inline
#endif
    std::vector<Partition>
    simulated_annealing(int k, int el, std::string hf_type, int source,
                        int goal, HeuristicFuncBase *hfunc, VisibilityFunc *vf,
                        std::vector<std::vector<std::pair<int, int>>> *graph,
                        std::vector<std::vector<int>> *asaplookup,
                        std::vector<int> &transit_candidates,
                        bool complete_search, bool use_upperbound_cost,
                        Logger &logger,
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

  int best_nap = 0;
  int prev_nap = 0;
  float best_mac = (float)MAX_DIST;
  float prev_mac = 0;
  bool valid_already_found = false;
  // flat_hash_set<size_t> checked_partitions;

  std::mt19937 gen(42);

  int itr = 0;
  while (true) {
    itr++;

    // ------ Finish --------
    // size_t hash_val = hash_values_of_partitions(partitions);
    // checked_partitions.insert(hash_val);
    logger.cum_count++;
    if (logger.print()) {
      break;
    }

    // ----- Pick New Solutions -----
    int partitions_num = partitions.size();
    std::vector<Partition *> next_partitions;
    std::vector<int> new_partitions_pos;
    std::vector<int> erase_partitions_pos;

    double p = (double)gen() / RAND_MAX;
    bool skip_flag = false;
    if (partitions_num > 1 && p > 0.7) {
      std::pair<int, int> ij = selectTwoRandomNumbers(partitions_num, gen);

      double r = (double)gen() / RAND_MAX;
      if (r > 0.5 && (partitions[ij.first]->elements.size() > 1) &&
          (partitions[ij.second]->elements.size() > 1)) {
        // random swap
        std::pair<Partition *, Partition *> pij =
            partitions[ij.first]->merge_split(partitions[ij.second], gen);
        next_partitions = partitions;
        next_partitions[ij.first] = pij.first;
        next_partitions[ij.second] = pij.second;

        new_partitions_pos.emplace_back(ij.first);
        new_partitions_pos.emplace_back(ij.second);
        erase_partitions_pos.emplace_back(ij.first);
        erase_partitions_pos.emplace_back(ij.second);

        pij.first->cover_path.clear();
        pij.first->cover_path.shrink_to_fit();
        pij.second->cover_path.clear();
        pij.second->cover_path.shrink_to_fit();
      } else {
        // merge
        Partition *partition_i_j =
            partitions[ij.first]->merge(partitions[ij.second], MAX_DIST);
        logger.total_num_expanded_node += partition_i_j->num_expanded_nodes;

        partition_i_j->cover_path.clear();
        partition_i_j->cover_path.shrink_to_fit();

        next_partitions.emplace_back(partition_i_j);
        new_partitions_pos.emplace_back(0);
        erase_partitions_pos.emplace_back(ij.first);
        erase_partitions_pos.emplace_back(ij.second);

        for (int w = 0; w < partitions_num; w++) {
          if (w != ij.first && w != ij.second) {
            next_partitions.emplace_back(partitions[w]);
          }
        }
      }

    } else {
      //  split
      std::uniform_int_distribution<> dis(0, partitions_num - 1);
      int i = dis(gen);
      if (partitions[i]->elements.size() > 1) {
        std::pair<Partition *, Partition *> ps =
            partitions[i]->random_split(itr);
        next_partitions.emplace_back(ps.first);
        next_partitions.emplace_back(ps.second);
        new_partitions_pos.emplace_back(0);
        new_partitions_pos.emplace_back(1);
        erase_partitions_pos.emplace_back(i);

        ps.first->cover_path.clear();
        ps.first->cover_path.shrink_to_fit();
        ps.second->cover_path.clear();
        ps.second->cover_path.shrink_to_fit();
        for (int w = 0; w < partitions_num; w++) {
          if (w != i) {
            next_partitions.emplace_back(partitions[w]);
          }
        }
      } else {
        next_partitions = partitions;
        skip_flag = true;
      }
    }

    if (skip_flag) {
      continue;
    }

    // ----- Evaluation -------

    bool valid_paritions = true;
    int satisfying_nap = 0;
    float sum_ac = 0;
    float satisfying_mac = 0;
    for (Partition *p : next_partitions) {
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
      best_partitions.reserve(next_partitions.size());
      for (Partition *p : next_partitions) {
        best_partitions.push_back(*p);
      }

      logger.sum_card = best_nap;
      logger.avg_path_cost = best_mac;
    }

    if (itr == 1) {
      prev_nap = satisfying_nap;
      prev_mac = satisfying_mac;
    }

    double q = (double)gen() / RAND_MAX;

    double temp = 1000; // start_temp + (end_temp - start_temp) * (now_time -
                        // start_time) / TIME_LIMIT;
    double prob = exp((prev_mac - satisfying_mac) / temp);
    if (((satisfying_nap > prev_nap) ||
         (satisfying_nap >= prev_nap && prev_mac > satisfying_mac)) ||
        (q > prob)) {
      for (int t : erase_partitions_pos) {
        delete partitions[t];
      }
      partitions = next_partitions;
      prev_nap = satisfying_nap;
      prev_mac = satisfying_mac;
    } else {
      for (int t : new_partitions_pos) {
        delete next_partitions[t];
      }
    }

    if (valid_paritions) {
      if (!valid_already_found) {
        logger.num_evaluated_partitions_till_first_solution = logger.cum_count;
        logger.num_expanded_node_till_first_solution =
            logger.total_num_expanded_node;
        valid_already_found = true;
      }
      logger.valid_count++;
    }
  }

  logger.summary();
  return best_partitions;
}
