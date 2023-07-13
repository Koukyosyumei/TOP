#pragma once
#include "covering_search.h"
#include "parallel_hashmap/phmap.h"
#include "utils.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using phmap::flat_hash_map;
using phmap::flat_hash_set;

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
  int h_to_unseen_min = MAX_DIST;
  int h_to_unseen_max = 0;
  int h_to_goal = MAX_DIST;

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
      h_to_unseen_min = MAX_DIST;
      h_to_unseen_max = 0;
      h_to_goal = asaplookup->at(source)[goal];
      return;
    }

    h_to_unseen_min = MAX_DIST;
    h_to_unseen_max = 0;
    std::vector<int> avp;
    for (int p : elements) {
      avp = vf->get_all_watchers(p);
      int tmp_min_h = MAX_DIST;
      for (int q : avp) {
        int tmp_cost = asaplookup->at(source)[q];
        tmp_min_h = std::min(tmp_min_h, tmp_cost);
      }
      h_to_unseen_min = std::min(tmp_min_h, h_to_unseen_min);
      h_to_unseen_max = std::max(tmp_min_h, h_to_unseen_max);
    }

    h_to_goal = MAX_DIST;
    for (int p : elements) {
      h_to_goal = std::min(h_to_goal, asaplookup->at(p)[goal]);
    }
  }

  void print_element(std::ofstream &log_file) {
    for (int e : elements) {
      log_file << e << " ";
    }
    log_file << std::endl;
  }

  void print_cover_path(std::ofstream &log_file) {
    for (const Node &node : cover_path) {
      log_file << node.location << " ";
      for (int p : node.unseen) {
        log_file << p << " ";
      }
      log_file << std::endl;
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
    par.h_to_unseen_min = std::min(h_to_unseen_min, other.h_to_unseen_min);
    par.h_to_unseen_max = std::max(h_to_unseen_max, other.h_to_unseen_max);
    par.h_to_goal = std::min(h_to_goal, other.h_to_goal);
    return par;
  }

  int dist(const Partition &rhs) {
    int dist = MAX_DIST;
    for (int i : elements) {
      for (int j : rhs.elements) {
        dist = std::min(dist, asaplookup->at(i)[j]);
        dist = std::min(dist, asaplookup->at(i)[j]);
      }
    }
    return dist;
  }

  int pathmatch(const Partition &rhs) {
    flat_hash_set<int> this_path_set, rhs_path_set;
    for (const Node &i : cover_path) {
      this_path_set.emplace(i.location);
    }
    for (const Node &j : rhs.cover_path) {
      rhs_path_set.emplace(j.location);
    }

    int mis_this = 0;
    int mis_rhs = 0;
    for (int i : elements) {
      if (rhs_path_set.find(i) == rhs_path_set.end()) {
        mis_this++;
      }
    }
    for (int j : rhs.elements) {
      if (this_path_set.find(j) == this_path_set.end()) {
        mis_rhs++;
      }
    }

    // return std::min(mis_this, mis_rhs);
    return mis_this + mis_rhs;
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
                                    Partition &p_j,
                                    bool is_cur_containing_pj = true) {
  size_t h_val_i = 0;
  size_t h_val_j = 0;
  for (int e : p_i.elements) {
    h_val_i ^= std::hash<std::string>{}(std::to_string(e));
  }
  if (is_cur_containing_pj) {
    for (int e : p_j.elements) {
      h_val_j ^= std::hash<std::string>{}(std::to_string(e));
    }
  }
  return cur_hash_val ^ std::hash<std::string>{}(std::to_string(h_val_i)) ^
         std::hash<std::string>{}(std::to_string(h_val_j)) ^
         std::hash<std::string>{}(std::to_string(h_val_i ^ h_val_j));
}

struct Logger {
  std::chrono::system_clock::time_point start_time, end_time;
  float verbose_interval, timeout;
  long long sum_card = 0;
  float avg_path_cost = 0;
  float num_print_called = 1;
  long long cum_count = 0;
  long long num_evaluated_partitions_till_first_solution = 0;
  long long num_expanded_node_till_first_solution = 0;
  long long total_num_expanded_node = 0;
  long long skipped_count = 0;
  long long duplicated_count = 0;
  long long valid_count = 0;
  long long not_promissing_count = 0;

  std::ofstream log_file;

  Logger(float verbose_interval_, float timeout_, std::string log_file_path)
      : start_time(std::chrono::system_clock::now()),
        verbose_interval(verbose_interval_), timeout(timeout_) {
    log_file.open(log_file_path, std::ios::out);
  }

  bool print(bool force = false) {
    end_time = std::chrono::system_clock::now();
    float elasped = std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time - start_time)
                        .count();
    if (force || elasped >= num_print_called * verbose_interval) {
      log_file << (num_print_called * verbose_interval) << " [ms], " << sum_card
               << " Anonymized Paths, " << avg_path_cost << " (Aveage Cost), "
               << cum_count << " Partitions Evaluated, " << valid_count
               << " Valid Partitions, " << skipped_count << " Skipped, "
               << duplicated_count << " Duplicated, " << total_num_expanded_node
               << " Node Expanded\n";
      num_print_called++;
    }

    return elasped >= timeout;
  }
  void summary() {
    log_file << "Performance Summary of DFBB:\n";
    log_file << "- Number of Evaluated Partitions in Total: " << cum_count
             << "\n";
    log_file << "- Number of Valid Partitions in Total: " << valid_count
             << "\n";
    log_file << "- Number of Skipped Partitions in Total: " << skipped_count
             << "\n";
    log_file << "- Number of Expanded Nodes in Total: "
             << total_num_expanded_node << "\n";
    log_file << "- Number of Evaluted Partitions to Find the First Satisfying "
                "Solution: "
             << num_evaluated_partitions_till_first_solution << "\n";
    log_file << "- Number of Expanded Nodes to Find the First Satisfying "
                "Solution: "
             << num_expanded_node_till_first_solution << "\n";
    log_file << "- Number of Duplicated Partitions in Total: "
             << duplicated_count << "\n";
    log_file << "- Number of Anonymized Paths: " << sum_card << "\n";
    log_file << "- Average Cost of Anonymized Paths: " << avg_path_cost << "\n";
    log_file << "- Time: "
             << std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time)
                    .count()
             << " [ms]\n";
  }

  void close() { log_file.close(); }
};

inline bool is_prunable(Partition &p_i, Partition &p_j, int sumcost,
                        flat_hash_set<size_t> &checked_partitions,
                        size_t hash_val, Logger &logger, int &best_sumcard,
                        int &best_sumcost, std::string hf_type, int k, int el,
                        bool complete_search, bool &valid_already_found,
                        bool use_upperbound_cost,
                        bool use_duplication_detection = true) {
  if (use_duplication_detection &&
      checked_partitions.find(hash_values_from_diff(hash_val, p_i, p_j)) !=
          checked_partitions.end()) {
    logger.duplicated_count++;
    return true;
  }

  if (p_i.is_satisfying && p_j.is_satisfying) {
    logger.skipped_count++;
    return true;
  }
  int upperbound_cost = MAX_DIST;
  if (valid_already_found && use_upperbound_cost) {
    upperbound_cost =
        best_sumcost -
        (sumcost - (int)p_i.elements.size() * p_i.cost_of_cover_path -
         (int)p_j.elements.size() * p_j.cost_of_cover_path);
    upperbound_cost /= ((int)p_i.elements.size() + (int)p_j.elements.size());

    int estimated_cost = 0;
    if (hf_type == "tunnel" || hf_type == "mst") {
      estimated_cost = std::max(p_i.h_to_unseen_max, p_j.h_to_unseen_max) +
                       std::min(p_i.h_to_goal, p_j.h_to_goal);
    } else if (hf_type == "tunnel+") {
      estimated_cost =
          std::max(std::max(p_i.h_to_unseen_max, p_j.h_to_unseen_max),
                   std::min(p_i.h_to_unseen_min, p_j.h_to_unseen_min) +
                       el * ((int)p_i.elements.size() +
                             (int)p_j.elements.size() - 1)) +
          std::min(p_i.h_to_goal, p_j.h_to_goal);
    }

    if (estimated_cost > upperbound_cost) {
      logger.skipped_count++;
      return true;
    }
  }

  int dist_between_i_j = p_i.dist(p_j);
  if (dist_between_i_j < el || dist_between_i_j == MAX_DIST) {
    logger.skipped_count++;
    return true;
  }

  return false;
}
