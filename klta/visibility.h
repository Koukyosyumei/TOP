#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "parallel_hashmap/phmap.h"
#include "utils.h"

using phmap::flat_hash_set;

struct VisibilityFunc {
  std::vector<std::vector<int>> graph;
  std::vector<std::vector<int>> asaplookup;
  VisibilityFunc(std::vector<std::vector<int>> &graph_,
                 std::vector<std::vector<int>> &asaplookup_) {
    graph = graph_;
    asaplookup = asaplookup_;
#ifdef _AF
    avp.reserve(1); // XXXTODO: must reserve enough to hold all watchers according to visibility function
    avp2.reserve(1); // XXXTODO: must reserve enough to hold all watchers according to visibility function    
#endif
  }
  virtual std::vector<int> get_all_visible_points(int i) = 0;
  virtual std::vector<int> get_all_watchers(int i) = 0;
#ifdef _AF
  virtual int get_all_visible_points2(int i, std::vector<int> &avp) = 0;
  virtual int get_all_watchers2(int i, std::vector<int> &avp) = 0;
  std::vector<int> avp;
    std::vector<int> avp2; 
#endif  
};

struct IdentityVF : public VisibilityFunc {
  using VisibilityFunc::VisibilityFunc;
  std::vector<int> get_all_visible_points(int i) { return {i}; }
  std::vector<int> get_all_watchers(int i) { return {i}; }
#ifdef _AF
  int get_all_visible_points2(int i, std::vector<int> &avp) { avp[0]=i; return 1; }  
  int get_all_watchers2(int i, std::vector<int> &avp) { avp[0]=i; return 1;}
#endif
};

struct RadiusVF : public VisibilityFunc {
  int radius;
  std::unordered_map<int, std::vector<int>> cache_av;
  std::unordered_map<int, std::vector<int>> cache_aw;
  RadiusVF(int radius_, std::vector<std::vector<int>> &graph_,
           std::vector<std::vector<int>> &asaplookup_)
      : radius(radius_), VisibilityFunc(graph_, asaplookup_) {}
  std::vector<int> get_all_visible_points(int i) {
    auto it = cache_av.find(i);
    if (it != cache_av.end()) {
      return it->second;
    }
    std::vector<int> result;
    for (int j = 0; j < graph.size(); j++) {
      if (asaplookup[i][j] <= radius) {
        result.push_back(j);
      }
    }
    result.push_back(i);
    cache_av[i] = result;
    return result;
  }
  std::vector<int> get_all_watchers(int i) {
    auto it = cache_aw.find(i);
    if (it != cache_aw.end()) {
      return it->second;
    }

    std::vector<int> result;
    for (int j = 0; j < graph.size(); j++) {
      if (asaplookup[j][i] <= radius) {
        result.push_back(j);
      }
    }
    result.push_back(i);
    cache_aw[i] = result;
    return result;
  }
#ifdef _AF
  int get_all_visible_points2(int i, std::vector<int> &avp) { assert(0); avp[0]=i; return 1; }  // dummy virtual  
  int get_all_watchers2(int i, std::vector<int> &avp) { assert(0); avp[0]=i; return 1;} // dummy virtual
#endif  
};

struct OneStepVF : public VisibilityFunc {
  using VisibilityFunc::VisibilityFunc;
  std::vector<int> get_all_visible_points(int i) {
    std::vector<int> result;
    for (int j = 0; j < graph.size(); j++) {
      if (graph[i][j] != MAX_DIST) {
        result.push_back(j);
      }
    }
    result.push_back(i);
    return result;
  }
  std::vector<int> get_all_watchers(int i) {
    std::vector<int> result;
    for (int j = 0; j < graph.size(); j++) {
      if (graph[j][i] != MAX_DIST) {
        result.push_back(j);
      }
    }
    result.push_back(i);
    return result;
  }
#ifdef _AF
  int get_all_visible_points2(int i, std::vector<int> &avp) { assert(0); avp[0]=i; return 1; }  // dummy virtual
  int get_all_watchers2(int i, std::vector<int> &avp) { assert(0); avp[0]=i; return 1;} // dummy virtual
#endif    
};

inline std::vector<int>
#ifdef _AF
choose_maximal_set_of_los_disjoint(VisibilityFunc *vf,
                                   Unseen &unseen, int cur) {
#else
choose_maximal_set_of_los_disjoint(VisibilityFunc *vf,
                                   flat_hash_set<int> &unseen, int cur) {
#endif
  std::vector<int> pivots;
#ifdef _AF
  assert(0); // XXXTODO:should use targetset, shouldn't loop 0<n<N
  for (int u = 0; u < N; u++) {
    if (unseen[u]==1) {
      bool disjoint = true;
      std::vector<int> u_los = vf->get_all_watchers(u);
      flat_hash_set<int> u_los_set(u_los.begin(), u_los.end());
      for (int p : pivots) {
	std::vector<int> p_los = vf->get_all_watchers(p);
	for (int q : p_los) {
	  if (u_los_set.find(q) != u_los_set.end()) {
	    disjoint = false;
	    break;
	  }
	}
      }
      if (disjoint) {
	pivots.push_back(u);
      }
    }
  }
#else
  for (int u : unseen) {
    bool disjoint = true;
    std::vector<int> u_los = vf->get_all_watchers(u);
    flat_hash_set<int> u_los_set(u_los.begin(), u_los.end());
    for (int p : pivots) {
      std::vector<int> p_los = vf->get_all_watchers(p);
      for (int q : p_los) {
        if (u_los_set.find(q) != u_los_set.end()) {
          disjoint = false;
          break;
        }
      }
    }
    if (disjoint) {
      pivots.push_back(u);
    }
  }
#endif
  return pivots;
}

inline std::vector<int> choose_frontier_watcher(VisibilityFunc *vf, int cur,
                                                int pivot) {
  std::vector<int> watchers = vf->get_all_watchers(pivot);
  std::vector<bool> is_component = std::vector<bool>(vf->graph.size());
  for (int w : watchers) {
    is_component[w] = true;
  }
  is_component[pivot] = true;

  std::vector<int> frontier_wathcers;
  for (int w : watchers) {
    bool is_internal = true;
    for (int i = 0; i < vf->graph.size(); i++) {
      if (vf->graph[i][w] != MAX_DIST && is_component[i]) {
        is_internal = false;
        break;
      }
    }
    if (!is_internal) {
      frontier_wathcers.push_back(w);
    }
  }
  return frontier_wathcers;
}
