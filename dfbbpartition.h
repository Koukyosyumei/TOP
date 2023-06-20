#pragma once
#include "covering_search.h"
#include "utils.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Partition {
  int k, el;
  int source, goal;
  HeuristicFuncBase *hfunc;
  VisibilityFunc *vf;
  std::vector<std::vector<int>> *graph;
  std::vector<std::vector<int>> *asaplookup;
  std::vector<int> elements;
  std::vector<Node> cover_path;
  int num_expanded_nodes = 0;
  int cost_of_cover_path;
  bool is_satisfying = true;
  bool is_cover_path_searched = false;
  int h_to_unseen = 0;
  int h_to_goal = INT_MAX;

  Partition(){};
  Partition(int k, int el, int source, int goal, HeuristicFuncBase *hfunc,
            VisibilityFunc *vf, std::vector<std::vector<int>> *graph,
            std::vector<std::vector<int>> *asaplookup,
            std::vector<int> &elements, int upperbound_cost)
      : k(k), el(el), source(source), goal(goal), hfunc(hfunc), vf(vf),
        graph(graph), asaplookup(asaplookup), elements(elements) {
    judge_path_covering_condition(upperbound_cost);
  }

  size_t hash_value() {
    size_t result = 0;
    for (int e : elements) {
      result ^= std::hash<std::string>{}(std::to_string(e));
    }

    return result;
  }

  void calculate_singleton_h_value() {
    if (elements.size() == 0) {
      h_to_unseen = 0;
      h_to_goal = asaplookup->at(source)[goal];
      return;
    }

    h_to_unseen = 0;
    std::vector<int> avp;
    for (int p : elements) {
      avp = vf->get_all_watchers(p);
      int tmp_min_h = INT_MAX;
      for (int q : avp) {
        int tmp_cost = asaplookup->at(source)[q];
        tmp_min_h = std::min(tmp_min_h, tmp_cost);
      }
      h_to_unseen = std::max(tmp_min_h, h_to_unseen);
    }

    h_to_goal = INT_MAX;
    for (int p : elements) {
      h_to_goal = std::min(h_to_goal, asaplookup->at(p)[goal]);
    }
  }

  void print_element() {
    for (int e : elements) {
      std::cout << e << " ";
    }
    std::cout << std::endl;
  }

  void print_cover_path() {
    for (const Node &node : cover_path) {
      std::cout << node.location << " ";
      for (int p : node.unseen) {
        std::cout << p << " ";
      }
      std::cout << std::endl;
    }
  }

  bool is_satisfy_el() {
    for (int i = 0; i < elements.size(); i++) {
      for (int j = 0; j < elements.size(); j++) {
        if (i != j) {
          if (asaplookup->at(elements[i])[elements[j]] < el) {
            return false;
          }
        }
      }
    }
    return true;
  }

  void judge_path_covering_condition(int upperbound_cost) {
    if (k > elements.size()) {
      is_satisfying = false;
      // return;
    }

    for (int i = 0; i < elements.size(); i++) {
      for (int j = 0; j < elements.size(); j++) {
        if (i != j) {
          if (asaplookup->at(elements[i])[elements[j]] < el) {
            is_satisfying = false;
            return;
            // break;
          }
        }
      }
    }

    std::pair<int, std::vector<Node>> search_result =
        search(hfunc, vf, source, goal, elements, upperbound_cost);
    num_expanded_nodes = search_result.first;
    cover_path = search_result.second;
    is_cover_path_searched = true;
    if (cover_path.size() == 0) {
      is_satisfying = false;
      return;
    }

    cost_of_cover_path = cover_path[cover_path.size() - 1].g;
  }

  Partition merge(Partition &other, int upperbound_cost) {
    std::vector<int> new_elements(elements);
    new_elements.insert(new_elements.end(), other.elements.begin(),
                        other.elements.end());

    Partition par = Partition(k, el, source, goal, hfunc, vf, graph, asaplookup,
                              new_elements, upperbound_cost);
    par.h_to_unseen = std::max(h_to_unseen, other.h_to_unseen);
    par.h_to_goal = std::min(h_to_goal, other.h_to_goal);
    return par;
  }

  int dist(const Partition &rhs) {
    int dist = INT_MAX;
    for (int i : elements) {
      for (int j : rhs.elements) {
        dist = std::min(dist, asaplookup->at(i)[j]);
        dist = std::min(dist, asaplookup->at(i)[j]);
      }
    }
    return dist;
  }

  int pathdist(const Partition &rhs) {
    int dist = INT_MAX;
    for (const Node &i : cover_path) {
      for (const Node &j : rhs.cover_path) {
        dist = std::min(dist, asaplookup->at(i.location)[j.location]);
      }
    }
    return dist;
  }
};

inline size_t hash_values_of_partitions(std::vector<Partition> &partitions) {
  size_t value = 0;
  for (Partition &p : partitions) {
    value ^= std::hash<std::string>{}(std::to_string(p.hash_value()));
  }

  return value;
}

template <typename T>
inline std::vector<size_t> argsort(const std::vector<T> &values) {
  std::vector<size_t> indices(values.size());
  std::iota(indices.begin(), indices.end(), 0);

  std::sort(indices.begin(), indices.end(),
            [&values](size_t i, size_t j) { return values[i] < values[j]; });

  return indices;
}

inline size_t hash_values_from_diff(size_t cur_hash_val, Partition &p_i,
                                    Partition &p_j) {
  size_t h_val_i = 0;
  size_t h_val_j = 0;
  for (int e : p_i.elements) {
    h_val_i ^= std::hash<std::string>{}(std::to_string(e));
  }
  for (int e : p_j.elements) {
    h_val_j ^= std::hash<std::string>{}(std::to_string(e));
  }
  return cur_hash_val ^ std::hash<std::string>{}(std::to_string(h_val_i)) ^
         std::hash<std::string>{}(std::to_string(h_val_j)) ^
         std::hash<std::string>{}(std::to_string(h_val_i ^ h_val_j));
}

struct Logger {
  long long cum_count = 0;
  long long num_evaluated_partitions_till_first_solution = 0;
  long long num_expanded_node_till_first_solution = 0;
  long long total_num_expanded_node = 0;
  long long skipped_count = 0;
  long long duplicated_count = 0;
  long long valid_count = 0;
  long long not_promissing_count = 0;
  void print() {
    std::cout << cum_count << " Partitions Evaluated (" << valid_count
              << " Valid Partitions) (" << skipped_count << " Skipped) ("
              << duplicated_count << " Duplicated) (" << total_num_expanded_node
              << " Node Expanded)\n";
  }
  void summary() {
    std::cout << "Performance Summary of DFBB:\n";
    std::cout << "- Number of Evaluated Partitions in Total: " << cum_count
              << "\n";
    std::cout << "- Number of Valid Partitions in Total: " << valid_count
              << "\n";
    std::cout << "- Number of Skipped Partitions in Total: " << skipped_count
              << "\n";
    std::cout << "- Number of Expanded Nodes in Total: "
              << total_num_expanded_node << "\n";
    std::cout << "- Number of Evaluted Partitions to Find the First Satisfying "
                 "Solution: "
              << num_evaluated_partitions_till_first_solution << "\n";
    std::cout << "- Number of Expanded Nodes to Find the First Satisfying "
                 "Solution: "
              << num_expanded_node_till_first_solution << "\n";
  }
};

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

    std::vector<size_t> j_order;

    std::vector<std::tuple<int, int, int, int>> dist_from_i;
    if (j_order_type == "nearest") {
      for (int j = i + 1; j < partitions_num; j++) {
        int upperbound_cost = INT_MAX;
        if (valid_already_found && use_upperbound_cost) {
          upperbound_cost =
              best_sumcost - (sumcost - partitions[i].cost_of_cover_path -
                              partitions[j].cost_of_cover_path);
          upperbound_cost /=
              (partitions[i].elements.size() + partitions[j].elements.size());
          // upperbound_cost += 1;
        }
        dist_from_i.push_back(
            std::make_tuple((int)(std::max(partitions[i].h_to_unseen,
                                           partitions[j].h_to_unseen) +
                                      std::min(partitions[i].h_to_goal,
                                               partitions[j].h_to_goal) >
                                  upperbound_cost),
                            0, 0, 0));
      }
      j_order = argsort(dist_from_i);
    }
    if (j_order_type == "nearestdist") {
      for (int j = i + 1; j < partitions_num; j++) {
        int upperbound_cost = INT_MAX;
        if (valid_already_found && use_upperbound_cost) {
          upperbound_cost =
              best_sumcost - (sumcost - partitions[i].cost_of_cover_path -
                              partitions[j].cost_of_cover_path);
          upperbound_cost /=
              (partitions[i].elements.size() + partitions[j].elements.size());
          // upperbound_cost += 1;
        }
        int tmp_dist = partitions[i].dist(partitions[j]);
        dist_from_i.push_back(
            std::make_tuple((int)(std::max(partitions[i].h_to_unseen,
                                           partitions[j].h_to_unseen) +
                                      std::min(partitions[i].h_to_goal,
                                               partitions[j].h_to_goal) >
                                  upperbound_cost),
                            (int)tmp_dist < el, tmp_dist, 0));
      }
      j_order = argsort(dist_from_i);
    } else if (j_order_type == "pathnearest") {
      for (int j = i + 1; j < partitions_num; j++) {
        dist_from_i.push_back(std::make_tuple(
            partitions[j].is_satisfying, partitions[i].pathdist(partitions[j]),
            partitions[j].elements.size(), 0));
      }
      j_order = argsort(dist_from_i);
    } else {
      j_order.resize(partitions_num - i - 1);
      std::iota(j_order.begin(), j_order.end(), 0);
    }

    for (int j : j_order) {
      if (j_order_type == "nearest" || j_order_type == "nearestdist") {
        if (std::get<0>(dist_from_i[j]) == 1 ||
            std::get<1>(dist_from_i[j]) == 1) {
          break;
        }
      }
      j += i + 1;
      if (checked_partitions.find(
              hash_values_from_diff(hash_val, partitions[i], partitions[j])) !=
          checked_partitions.end()) {
        logger.duplicated_count++;
        continue;
      }

      if (partitions[i].is_satisfying && partitions[j].is_satisfying) {
        logger.skipped_count++;
        continue;
      }

      if ((partitions[i].is_cover_path_searched &&
           partitions[i].cover_path.size() == 0) ||
          (partitions[j].is_cover_path_searched &&
           partitions[j].cover_path.size() == 0)) {
        logger.skipped_count++;
        continue;
      }

      int upperbound_cost = INT_MAX;
      if (valid_already_found && use_upperbound_cost) {
        upperbound_cost =
            best_sumcost - (sumcost - partitions[i].cost_of_cover_path -
                            partitions[j].cost_of_cover_path);
        upperbound_cost /=
            (partitions[i].elements.size() + partitions[j].elements.size());
        // upperbound_cost += 1;
      }
      Partition partition_i_j =
          partitions[i].merge(partitions[j], upperbound_cost);
      logger.total_num_expanded_node += partition_i_j.num_expanded_nodes;

      if (!partition_i_j.is_satisfy_el()) {
        logger.skipped_count++;
        continue;
      }

      // std::cout << partition_i_j.cost_of_cover_path << " " << upperbound_cost
      //          << std::endl;
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
