#pragma once

#include "mst.h"
#include "parallel_hashmap/phmap.h"
#include "utils.h"
#include "visibility.h"

using phmap::flat_hash_map;
using phmap::flat_hash_set;

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
#ifdef _AF
  virtual int calculate_hval(Node &node, std::vector<int> &target_elements) = 0;
#else
  virtual int calculate_hval(Node &node) = 0;
#endif
};

struct BlindHeuristic : public HeuristicFuncBase {
  BlindHeuristic(VisibilityFunc *vf_,
                 std::vector<std::vector<int>> &asaplookup_, int goal_)
      : HeuristicFuncBase(vf_, asaplookup_, goal_) {}
#ifdef _AF
  int calculate_hval(Node &node, std::vector<int> &target_elements) {
    return 0;
  }
#else
  int calculate_hval(Node &node) { return 0; }
#endif
};

struct TunnelHeuristic : public HeuristicFuncBase {
  TunnelHeuristic(VisibilityFunc *vf_,
                  std::vector<std::vector<int>> &asaplookup_, int goal_)
      : HeuristicFuncBase(vf_, asaplookup_, goal_) {}
#ifdef _AF
  int calculate_hval(Node &node, std::vector<int> &target_elements){
#else
  int calculate_hval(Node &node) {
#endif

#ifdef _AF
      if (node.numunseen == 0) {
        return asaplookup[node.location][goal];
}
#else
    if (node.unseen.size() == 0) {
      return asaplookup[node.location][goal];
    }
#endif
int h_to_unseen = 0;
#ifdef _AF
std::vector<int> &avp = vf->avp;
for (int p : target_elements) {
  if (node.unseen[p] == 1) {
    int num_watchers = vf->get_all_watchers2(p, avp);
    int tmp_min_h = MAX_DIST;
    for (int i = 0; i < num_watchers; i++) {
      tmp_min_h = std::min(tmp_min_h, asaplookup[node.location][avp[i]]);
    }
    h_to_unseen = std::max(tmp_min_h, h_to_unseen);
  }
}
#else
    std::vector<int> avp;
    for (int p : node.unseen) {
      avp = vf->get_all_watchers(p);
      int tmp_min_h = MAX_DIST;
      for (int q : avp) {
        int tmp_cost = asaplookup[node.location][q];
        tmp_min_h = std::min(tmp_min_h, tmp_cost);
      }
      h_to_unseen = std::max(tmp_min_h, h_to_unseen);
    }
#endif
int h_to_goal = MAX_DIST;
#ifdef _AF
for (int p : target_elements) {
  if (node.unseen[p] == 1) {
    int num_watchers = vf->get_all_watchers2(p, avp);
    for (int i = 0; i < num_watchers; i++) {
      h_to_goal = std::min(h_to_goal, asaplookup[avp[i]][goal]);
    }
  }
}
#else
    for (int p : node.unseen) {
      avp = vf->get_all_watchers(p);
      for (int q : avp) {
        h_to_goal = std::min(h_to_goal, asaplookup[q][goal]);
      }
    }
#endif
return h_to_unseen + h_to_goal;
}
}
;

struct TunnelPlusHeuristic : public HeuristicFuncBase {
  int el;
  TunnelPlusHeuristic(VisibilityFunc *vf_,
                      std::vector<std::vector<int>> &asaplookup_, int goal_,
                      int el_)
      : HeuristicFuncBase(vf_, asaplookup_, goal_), el(el_) {}
#ifdef _AF
  int calculate_hval(Node &node, std::vector<int> &target_elements){
#else
  int calculate_hval(Node &node) {
#endif

#ifdef _AF
      if (node.numunseen == 0) {
        return asaplookup[node.location][goal];
}
#else
    if (node.unseen.size() == 0) {
      return asaplookup[node.location][goal];
    }
#endif
int h_to_unseen_min = MAX_DIST;
int h_to_unseen_max = 0;
std::vector<int> avp;
#ifdef _AF
for (int p : target_elements) {
  if (node.unseen[p] == 1) {
    avp = vf->get_all_watchers(p);
    int tmp_min_h = MAX_DIST;
    for (int q : avp) {
      int tmp_cost = asaplookup[node.location][q];
      tmp_min_h = std::min(tmp_min_h, tmp_cost);
    }
    h_to_unseen_min = std::min(tmp_min_h, h_to_unseen_min);
    h_to_unseen_max = std::max(tmp_min_h, h_to_unseen_max);
  }
}
#else
    for (int p : node.unseen) {
      avp = vf->get_all_watchers(p);
      int tmp_min_h = MAX_DIST;
      for (int q : avp) {
        int tmp_cost = asaplookup[node.location][q];
        tmp_min_h = std::min(tmp_min_h, tmp_cost);
      }
      h_to_unseen_min = std::min(tmp_min_h, h_to_unseen_min);
      h_to_unseen_max = std::max(tmp_min_h, h_to_unseen_max);
    }
#endif
int h_to_goal = MAX_DIST;
#ifdef _AF
for (int p : target_elements) {
  if (node.unseen[p] == 1) {
    avp = vf->get_all_watchers(p);
    for (int q : avp) {
      h_to_goal = std::min(h_to_goal, asaplookup[q][goal]);
    }
  }
}
#else
    for (int p : node.unseen) {
      avp = vf->get_all_watchers(p);
      for (int q : avp) {
        h_to_goal = std::min(h_to_goal, asaplookup[q][goal]);
      }
    }
#endif

#ifdef _AF
return std::max(h_to_unseen_max,
                h_to_unseen_min + el * ((int)node.numunseen - 1)) +
       h_to_goal;
#else
    return std::max(h_to_unseen_max,
                    h_to_unseen_min + el * ((int)node.unseen.size() - 1)) +
           h_to_goal;
#endif
}
}
;

/*
struct MSTHeuristic : public HeuristicFuncBase {
  using HeuristicFuncBase::HeuristicFuncBase;
#ifdef _AF
  int calculate_hval(Node &node, std::vector<int> &target_elements) {
#else
  int calculate_hval(Node &node) {
#endif

#ifdef _AF
    if (node.numunseen == 0) {
      return asaplookup[node.location][goal];
    }
#else
    if (node.unseen.size() == 0) {
      return asaplookup[node.location][goal];
    }
#endif
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
    std::vector<Edge> edges;

    int pivots_id = 0;
    int counter = 0;
    for (int i = 0; i < pivots_num; i++) {
      counter++;
      pivots_id = counter;
      int watcher_num_of_pivpt_i = watchers[i].size();
      for (int j = 0; j < watcher_num_of_pivpt_i; j++) {
        counter++;
        Edge e1 = {pivots_id, counter, 0};
        Edge e2 = {0, counter,
                   std::min(asaplookup[node.location][watchers[i][j]],
                            asaplookup[watchers[i][j]][node.location])};
        edges.emplace_back(e1);
        edges.emplace_back(e2);
      }
    }
    h_mst = mst_cost(&edges, 1 + pivots_num + watchers_num);

    std::vector<int> avp;
    int h_to_goal = MAX_DIST;
#ifdef _AF
    for (int p : target_elements) {
      if(node.unseen[p]==1) {
        avp = vf->get_all_watchers(p);
        for (int q : avp) {
          h_to_goal = std::min(h_to_goal, asaplookup[q][goal]);
        }
      }
    }
#else
    for (int p : node.unseen) {
      avp = vf->get_all_watchers(p);
      for (int q : avp) {
        h_to_goal = std::min(h_to_goal, asaplookup[q][goal]);
      }
    }
#endif
    return h_mst + h_to_goal;
  }
};
*/
