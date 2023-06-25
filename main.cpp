#include <unistd.h>

#include "asap.h"
#include "covering_search.h"
#include "dfbbpartition.h"
#include "greedypartition.h"
#include "heuristic.h"
#include "visibility.h"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int k = 2;
int el = 0;
float verbose = 1000;
std::string partition_type = "merge";
std::string vf_type = "identity";
std::string hf_type = "blind";
std::string j_order_type = "random";
bool complete_search = false;
bool use_upperbound_cost = false;

void parse_args(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "k:l:v:p:h:j:b:cu")) != -1) {
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
    case 'c':
      complete_search = true;
      break;
    case 'u':
      use_upperbound_cost = true;
      break;
    default:
      printf("unknown parameter %s is specified", optarg);
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  std::cout << "Setup Started\n";

  parse_args(argc, argv);
  std::cout << ": k=" << k << "\n";
  std::cout << ": el=" << el << "\n";
  std::cout << ": v=" << vf_type << "\n";
  std::cout << ": h=" << hf_type << "\n";
  std::cout << ": j=" << j_order_type << "\n";
  std::cout << ": c=" << complete_search << "\n";
  std::cout << ": u=" << use_upperbound_cost << "\n";

  int N, E, source, goal, a, b;
  float c;
  std::cin >> N >> E;

  std::vector<std::vector<int>> graph(N, std::vector<int>(N, INT_MAX));

  for (int i = 0; i < E; i++) {
    std::cin >> a >> b >> c;
    graph[a][b] = c;
  }
  std::cin >> source >> goal;
  std::cout << " Input Loading Completed\n";

  std::vector<std::vector<int>> asaplookup = get_asaplookup(graph);
  std::cout << " ASAP Table Calculation Completed\n";

  VisibilityFunc *vf;
  if (vf_type == "identity") {
    vf = new IdentityVF(graph);
  } else if (vf_type == "onestep") {
    vf = new OneStepVF(graph);
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
  } else if (hf_type == "mst") {
    hf = new MSTHeuristic(vf, asaplookup, goal);
  } else {
    throw std::invalid_argument(
        "Heuristic function should be blind/tunnel/tunelidentity/mst.");
  }

  std::cout << "Setup Completed\n";
  std::cout << "Optimal Partition Search Started\n";

  std::vector<Partition> partitions;

  if (partition_type == "merge") {
    partitions =
        merge_df_bb(k, el, j_order_type, source, goal, hf, vf, &graph,
                    &asaplookup, complete_search, verbose, use_upperbound_cost);
  } else if (partition_type == "greedy") {
    partitions = greedypartition(k, el, j_order_type, source, goal, hf, vf,
                                 &graph, &asaplookup, complete_search, verbose,
                                 use_upperbound_cost);
  } else {
    throw std::invalid_argument("Partition type should be merge/greedy");
  }

  if (partitions.size() == 0) {
    std::cout << " No Satisfying Partition Found\n";
  } else {
    std::cout << " Displaying Optimal Partition...\n";
    int count = 0;
    for (Partition &partition : partitions) {
      std::cout << "Partition " << count << " (" << partition.num_expanded_nodes
                << " Nodes Expanded)\n";
      partition.print_element();
      partition.print_cover_path();
      count++;
    }
  }
}
