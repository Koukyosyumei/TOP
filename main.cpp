#include <unistd.h>

#define _AF 1
//#define _HASH2 // XXX Important: _AF MUST be defined in order to use _HASH2
#define _MERGE2 // alternate branching implementation for mergedfbb
#ifdef _AF
const int MAXPARTSIZE = 10000;
int N = 0;
#else // Prevent other modifications by AF from being compiled if _AF is not
      // defined.
#ifdef _HASH2
#error "preprocessor def _AF must be defined in order to use _HASH2  !!"
#endif
#ifdef _MERGE2
#error "preprocessor def _AF must be defined in order to use _MERGE2  !!"
#endif
#endif

#include "klta/asap.h"
#include "klta/covering_search.h"
#include <cassert>
#ifdef _MERGE2
#include "klta/merge2.h"
#endif
#include "klta/covering_search.h"
#include "klta/greedypartition.h"
#include "klta/heuristic.h"
#include "klta/mergebb.h"
#include "klta/utils.h"
#include "klta/visibility.h"
#include <chrono>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

int k = 2;
int el = 0;
int r = 1;
float verbose = 1000;
float timeout = std::numeric_limits<float>::max();
std::string partition_type = "merge";
std::string vf_type = "identity";
std::string hf_type = "blind";
std::string j_order_type = "random";
std::string log_file_path = "log.out";
bool complete_search = false;
bool use_upperbound_cost = false;
bool print_cover_path = false;

void parse_args(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "k:l:v:r:p:h:j:b:t:f:cua")) != -1) {
    switch (opt) {
    case 'k':
      k = atoi(optarg);
      break;
    case 'l':
      el = atoi(optarg);
      break;
    case 'v':
      vf_type = std::string(optarg);
      break;
    case 'r':
      r = std::stoi(optarg);
      break;
    case 'p':
      partition_type = std::string(optarg);
      break;
    case 'h':
      hf_type = std::string(optarg);
      break;
    case 'j':
      j_order_type = std::string(optarg);
      break;
    case 'b':
      verbose = atof(optarg);
      break;
    case 't':
      timeout = atof(optarg);
      break;
    case 'f':
      log_file_path = std::string(optarg);
    case 'c':
      complete_search = true;
      break;
    case 'u':
      use_upperbound_cost = true;
      break;
    case 'a':
      print_cover_path = true;
      break;
    default:
      printf("unknown parameter %s is specified", optarg);
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  parse_args(argc, argv);
  Logger logger(verbose, timeout, log_file_path);

  logger.log_file << "Setup Started\n";

  logger.log_file << ": k=" << k << "\n";
  logger.log_file << ": el=" << el << "\n";
  logger.log_file << ": v=" << vf_type << "\n";
  logger.log_file << ": h=" << hf_type << "\n";
  logger.log_file << ": j=" << j_order_type << "\n";
  logger.log_file << ": c=" << complete_search << "\n";
  logger.log_file << ": u=" << use_upperbound_cost << "\n";
  logger.log_file << ": p=" << partition_type << "\n";
  logger.log_file << ": r=" << r << "\n";

#ifdef _AF
  int E, source, goal, a, b;
#else
  int N, E, source, goal, a, b;
#endif
  float c;
  std::cin >> N >> E;
#ifdef _AF
  if (N > MAXPARTSIZE) {
    std::cout << "Error: N = " << N << " > "
              << "MAXPARTSIZE=" << MAXPARTSIZE << std::endl;
    std::cout << "Must recompile with MAXPARTSIZE >= N" << std::endl;
    exit(1);
  }
#endif
  std::vector<std::vector<std::pair<int, int>>> graph(N);

  for (int i = 0; i < E; i++) {
    std::cin >> a >> b >> c;
    graph[a].emplace_back(std::make_pair(b, c));
  }
  std::cin >> source >> goal;

  int num_transit_candidates = 0;
  std::cin >> num_transit_candidates;
  std::vector<int> transit_candidates(num_transit_candidates);
  for (int i = 0; i < num_transit_candidates; i++) {
    std::cin >> transit_candidates[i];
  }

  logger.log_file << " Input Loading Completed\n";

  std::vector<std::vector<int>> asaplookup = get_asaplookup(graph);
  logger.log_file << " ASAP Table Calculation Completed\n";

  VisibilityFunc *vf;
  if (vf_type == "identity") {
    vf = new IdentityVF(&graph, &asaplookup);
  } else if (vf_type == "onestep") {
    vf = new OneStepVF(&graph, &asaplookup);
  } else if (vf_type == "radius") {
    vf = new RadiusVF(r, &graph, &asaplookup);
  } else {
    throw std::invalid_argument(
        "Visibility function should be identity/onestep");
  }

  HeuristicFuncBase *hf;
  if (hf_type == "blind") {
    hf = new BlindHeuristic(vf, asaplookup, goal);
  } else if (hf_type == "tunnel") {
    hf = new TunnelHeuristic(vf, asaplookup, goal);
  } else if (hf_type == "tunnel+") {
    hf = new TunnelPlusHeuristic(vf, asaplookup, goal, el);
  } else {
    throw std::invalid_argument(
        "Heuristic function should be blind/tunnel/tunelidentity/mst.");
  }

  logger.log_file << "Setup Completed\n";

  flat_hash_map<int, int> base_dist_map;
  for (int t : transit_candidates) {
    int dist = MAX_DIST;
    for (int w : vf->get_all_watchers(t)) {
      dist = std::min(dist, asaplookup[source][w] + asaplookup[w][goal]);
    }
    base_dist_map[t] = dist;
  }

  logger.log_file << "Optimal Partition Search Started\n";
  std::vector<Partition> partitions;

  logger.start_timer();

  if (partition_type == "merge") {
    partitions =
        merge_df_bb(k, el, hf_type, j_order_type, source, goal, hf, vf, &graph,
                    &asaplookup, transit_candidates, complete_search,
                    use_upperbound_cost, logger, base_dist_map);
  } else if (partition_type == "df") {
    partitions = greedypartition(k, el, hf_type, j_order_type, source, goal, hf,
                                 vf, &graph, &asaplookup, transit_candidates,
                                 complete_search, use_upperbound_cost, logger,
                                 false, base_dist_map);
  } else if (partition_type == "df+") {
    partitions = greedypartition(k, el, hf_type, j_order_type, source, goal, hf,
                                 vf, &graph, &asaplookup, transit_candidates,
                                 complete_search, use_upperbound_cost, logger,
                                 true, base_dist_map);
  } else if (partition_type == "naive") {
    std::vector<int> feasible_transit_candidates;
    std::vector<int> visible_points_of_i;
    for (int i : transit_candidates) {
      if (i == source || i == goal) {
        continue;
      }

      bool is_valid = false;
      visible_points_of_i = vf->get_all_watchers(i);
      for (int j : visible_points_of_i) {
        if ((asaplookup.at(source)[j] != MAX_DIST) &&
            (asaplookup.at(j)[goal] != MAX_DIST)) {
          is_valid = true;
          break;
        }
      }
      if (is_valid) {
        feasible_transit_candidates.emplace_back(i);
      }
    }

    std::default_random_engine rng(42);
    std::shuffle(feasible_transit_candidates.begin(),
                 feasible_transit_candidates.end(), rng);

    logger.tot_node_num = transit_candidates.size();
    logger.log_file << transit_candidates.size() -
                           feasible_transit_candidates.size()
                    << " Nodes Removed\n";

    int num_partition = feasible_transit_candidates.size() / k;
    for (int j = 0; j < num_partition - 1; j++) {
      std::vector<int> elements;
      for (int i = j * k; i < (j + 1) * k; i++) {
        elements.push_back(feasible_transit_candidates[i]);
      }
      partitions.emplace_back(Partition(k, el, source, goal, hf, vf, &graph,
                                        &asaplookup, &base_dist_map, elements,
                                        MAX_DIST));
    }
    std::vector<int> elements;
    for (int i = k * (num_partition - 1);
         i < feasible_transit_candidates.size(); i++) {
      elements.emplace_back(feasible_transit_candidates[i]);
    }
    partitions.emplace_back(Partition(k, el, source, goal, hf, vf, &graph,
                                      &asaplookup, &base_dist_map, elements,
                                      MAX_DIST));

    float ac = 0;
    for (const Partition &p : partitions) {
      ac += p.ac;
    }

    logger.avg_path_cost = ac / (float)feasible_transit_candidates.size();
    logger.end_time = std::chrono::system_clock::now();
    logger.summary();
  } else if (partition_type == "wrp") {
    std::vector<int> feasible_transit_candidates;
    std::vector<int> visible_points_of_i;
    for (int i : transit_candidates) {
      if (i == source || i == goal) {
        continue;
      }

      bool is_valid = false;
      visible_points_of_i = vf->get_all_watchers(i);
      for (int j : visible_points_of_i) {
        if ((asaplookup.at(source)[j] != MAX_DIST) &&
            (asaplookup.at(j)[goal] != MAX_DIST)) {
          is_valid = true;
          break;
        }
      }
      if (is_valid) {
        feasible_transit_candidates.emplace_back(i);
      }
    }

    logger.tot_node_num = transit_candidates.size();
    logger.log_file << transit_candidates.size() -
                           feasible_transit_candidates.size()
                    << " Nodes Removed\n";

    std::pair<int, std::vector<Node>> result =
        search(hf, vf, source, goal, feasible_transit_candidates, MAX_DIST);
    int cost_of_cover_path = result.second[result.second.size() - 1].g;
    float ac = 0;
    for (int e : feasible_transit_candidates) {
      ac += ((float)cost_of_cover_path - (float)base_dist_map[e]) /
            (float)base_dist_map[e];
    }
    logger.avg_path_cost = ac / (float)feasible_transit_candidates.size();
    logger.end_time = std::chrono::system_clock::now();
    logger.summary();
  } else {
    throw std::invalid_argument("Partition type should be merge/df/df+");
  }

  float sum_dist_tmp = 0;
  float sum_card_tmp = 0;
  for (int i = 0; i < N; i++) {
    if ((i != source) && (i != goal) && (asaplookup[source][i] != MAX_DIST) &&
        (asaplookup[i][goal] != MAX_DIST)) {
      sum_dist_tmp += asaplookup[source][i] + asaplookup[i][goal];
      sum_card_tmp++;
    }
  }
  logger.log_file << "- Lowerbound Cost of Anonnymized Paths: "
                  << sum_dist_tmp / sum_card_tmp << "\n";

  if (partitions.size() == 0) {
    logger.log_file << " No Satisfying Partition Found\n";
  } else {
    logger.log_file << " Displaying Optimal Partition...\n";
    int count = 0;
    for (Partition &partition : partitions) {
      logger.log_file << "Partition " << count << " ("
                      << partition.num_expanded_nodes << " Nodes Expanded)\n";
      partition.print_element(logger.log_file);
      if (print_cover_path) {
        partition.cover_path =
            search(hf, vf, source, goal, partition.elements, MAX_DIST).second;
        partition.print_cover_path(logger.log_file);
      }
      count++;
    }
  }

  logger.close();
}
