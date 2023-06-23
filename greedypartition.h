#pragma once
#include "covering_search.h"
#include "partition.h"
#include "utils.h"

inline bool greedypartition_search(
    std::string j_order_type, std::vector<Partition> &best_partitions,
    std::vector<Partition> subsets, std::vector<Partition> unassigned,
    std::unordered_set<size_t> &checked_partitions, Logger &logger,
    int &best_sumcard, int &best_sumcost, int k, int el, bool complete_search,
    int log_count, bool &valid_already_found, bool use_upperbound_cost) {

  size_t hash_val = hash_values_of_partitions(subsets);
  checked_partitions.insert(hash_val);
  logger.cum_count++;

  if (logger.cum_count != 0 && logger.cum_count % log_count == 0) {
    logger.print();
  }

  bool valid_paritions = true;
  int sumcost = 0;
  int satisfying_sumcard = 0;
  int satisfying_sumcost = 0;
  for (Partition p : subsets) {
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
    best_partitions = subsets;

    std::cout << "Best Cardinarity: " << best_sumcard << ", Best AVG Cost: "
              << (float)best_sumcost / (float)best_sumcard << "\n";
  }

  if (unassigned.size() == 0) {

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
    Partition picked = unassigned[n];
    unassigned.erase(unassigned.begin() + n);
    for (int i = 0; i < subsets.size(); i++) {
      Partition p_tmp = subsets[i];

      if (is_prunable(picked, subsets[i], sumcost, checked_partitions, hash_val,
                      logger, best_sumcard, best_sumcost, k, el,
                      complete_search, log_count, valid_already_found,
                      use_upperbound_cost, false)) {
        continue;
      }

      subsets[i] = subsets[i].merge(picked, INT_MAX);
      logger.total_num_expanded_node += subsets[i].num_expanded_nodes;
      bool flag = greedypartition_search(
          j_order_type, best_partitions, subsets, unassigned,
          checked_partitions, logger, best_sumcard, best_sumcost, k, el,
          complete_search, log_count, valid_already_found, use_upperbound_cost);
      if (flag) {
        return true;
      }
      subsets[i] = p_tmp;
    }
    subsets.push_back(picked);
    bool flag = greedypartition_search(
        j_order_type, best_partitions, subsets, unassigned, checked_partitions,
        logger, best_sumcard, best_sumcost, k, el, complete_search, log_count,
        valid_already_found, use_upperbound_cost);
    if (flag) {
      return true;
    }
  }

  return false;
}

inline std::vector<Partition>
greedypartition(int k, int el, std::string j_order_type, int source, int goal,
                HeuristicFuncBase *hfunc, VisibilityFunc *vf,
                std::vector<std::vector<int>> *graph,
                std::vector<std::vector<int>> *asaplookup, bool complete_search,
                int verbose, bool use_upperbound_cost) {
  int N = graph->size();
  std::vector<Partition> best_partitions(0);
  std::vector<Partition> subsets;
  std::vector<Partition> unassigned;
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
      unassigned.push_back(Partition(k, el, source, goal, hfunc, vf, graph,
                                     asaplookup, tmp_elements, INT_MAX));
      unassigned[unassigned.size() - 1].calculate_singleton_h_value();
      logger.total_num_expanded_node +=
          unassigned[unassigned.size() - 1].num_expanded_nodes;
    }
  }

  std::cout << graph->size() - 2 - unassigned.size() << " Nodes Removed\n";

  int best_sumcard = 0;
  int best_sumcost = INT_MAX;
  bool valid_found = false;
  std::unordered_set<size_t> checked_partitions;
  greedypartition_search(j_order_type, best_partitions, subsets, unassigned,
                         checked_partitions, logger, best_sumcard, best_sumcost,
                         k, el, complete_search, verbose, valid_found,
                         use_upperbound_cost);
  logger.summary();
  std::cout << "- Number of Anonymized Paths: " << best_sumcard << "\n";
  std::cout << "- Average Cost of Anonymized Paths: "
            << (float)best_sumcost / (float)best_sumcard << "\n";
  return best_partitions;
}
