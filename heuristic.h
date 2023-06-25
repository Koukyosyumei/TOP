#pragma once

#include "mst.h"
#include "utils.h"
#include "visibility.h"

struct HeuristicFuncBase {
  VisibilityFunc *vf;
  int goal;
  std::vector<std::vector<int>> asaplookup;
  HeuristicFuncBase(VisibilityFunc *vf_,
                    std::vector<std::vector<int>> &asaplookup_, int goal_) {
    vf = vf_;
    asaplookup = asaplookup_;
    goal = goal_;
  }
  virtual int calculate_hval(Node &node) = 0;
};

struct BlindHeuristic : public HeuristicFuncBase {
  BlindHeuristic(VisibilityFunc *vf_,
                 std::vector<std::vector<int>> &asaplookup_, int goal_)
      : HeuristicFuncBase(vf_, asaplookup_, goal_) {}

  int calculate_hval(Node &node) { return 0; }
};

struct TunnelHeuristic : public HeuristicFuncBase {
  TunnelHeuristic(VisibilityFunc *vf_,
                  std::vector<std::vector<int>> &asaplookup_, int goal_)
      : HeuristicFuncBase(vf_, asaplookup_, goal_) {}

  int calculate_hval(Node &node) {
    if (node.unseen.size() == 0) {
      return asaplookup[node.location][goal];
    }

    int h_to_unseen = 0;
    std::vector<int> avp;
    for (int p : node.unseen) {
      avp = vf->get_all_watchers(p);
      int tmp_min_h = INT_MAX;
      for (int q : avp) {
        int tmp_cost = asaplookup[node.location][q];
        tmp_min_h = std::min(tmp_min_h, tmp_cost);
      }
      h_to_unseen = std::max(tmp_min_h, h_to_unseen);
    }

    int h_to_goal = INT_MAX;
    for (int p : node.unseen) {
      avp = vf->get_all_watchers(p);
      for (int q : avp) {
        h_to_goal = std::min(h_to_goal, asaplookup[q][goal]);
      }
    }

    return h_to_unseen + h_to_goal;
  }
};

struct TunnelIdentity : public HeuristicFuncBase {
  TunnelHeuristic tunnel_hf;
  int el;
  TunnelIdentity(VisibilityFunc *vf_,
                 std::vector<std::vector<int>> &asaplookup_, int goal_, int el_)
      : HeuristicFuncBase(vf_, asaplookup_, goal_),
        tunnel_hf(vf_, asaplookup_, goal_), el(el_) {}

  int calculate_hval(Node &node) {
    if (node.unseen.size() <= 1) {
      return tunnel_hf.calculate_hval(node);
    }
    return tunnel_hf.calculate_hval(node) + el * ((int)node.unseen.size() - 1);
  }
};

struct MSTHeuristic : public HeuristicFuncBase {
  using HeuristicFuncBase::HeuristicFuncBase;

  int calculate_hval(Node &node) {
    if (node.unseen.size() == 0) {
      return asaplookup[node.location][goal];
    }

    int h_mst = 0;
    std::vector<int> pivots =
        choose_maximal_set_of_los_disjoint(vf, node.unseen, node.location);
    std::vector<std::vector<int>> watchers;
    int watchers_num = 0;
    for (int p : pivots) {
      watchers.push_back(choose_frontier_watcher(vf, node.location, p));
      watchers_num += watchers[watchers.size() - 1].size();
    }

    int pivots_num = pivots.size();
    std::vector<std::vector<int>> adj_matrix = std::vector<std::vector<int>>(
        1 + pivots_num + watchers_num,
        std::vector<int>(1 + pivots_num + watchers_num, INT_MAX));

    int pivots_id = 0;
    int counter = 0;
    for (int i = 0; i < pivots_num; i++) {
      counter++;
      pivots_id = counter;
      int watcher_num_of_pivpt_i = watchers[i].size();
      for (int j = 0; j < watcher_num_of_pivpt_i; j++) {
        counter++;
        adj_matrix[pivots_id][counter] = asaplookup[pivots[i]][watchers[i][j]];
        adj_matrix[counter][pivots_id] = 0;
        adj_matrix[0][counter] = asaplookup[node.location][watchers[i][j]];
        adj_matrix[counter][0] = asaplookup[watchers[i][j]][node.location];
      }
    }
    h_mst = mst_cost(adj_matrix);

    std::vector<int> avp;
    int h_to_goal = INT_MAX;
    for (int p : node.unseen) {
      avp = vf->get_all_watchers(p);
      for (int q : avp) {
        h_to_goal = std::min(h_to_goal, asaplookup[q][goal]);
      }
    }
    return h_mst + h_to_goal;
  }
};
