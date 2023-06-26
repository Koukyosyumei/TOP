import argparse
import random

import networkx as nx


def add_args(parser):
    parser.add_argument(
        "-n",
        "--num_nodes",
        default=10,
        type=int,
    )
    parser.add_argument(
        "-e",
        "--edges_fraction",
        default=0.3,
        type=float,
    )
    parser.add_argument("-m", "--max_weight", default=1, type=int)
    parser.add_argument(
        "-s",
        "--seed",
        default=42,
        type=int,
    )
    parser.add_argument(
        "-d",
        "--is_directed",
        action="store_true",
    )

    args = parser.parse_args()
    return args


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parsed_args = add_args(parser)

    num_nodes = parsed_args.num_nodes
    edges_fraction = parsed_args.edges_fraction
    seed = parsed_args.seed
    is_directed = parsed_args.is_directed

    random.seed(seed)
    G = nx.random_graphs.fast_gnp_random_graph(
        num_nodes, edges_fraction, seed, directed=is_directed
    )

    edges = list(G.edges)
    if is_directed:
        print(num_nodes, len(edges))
    else:
        print(num_nodes, 2 * len(edges))
    for e in edges:
        w = random.randint(1, parsed_args.max_weight)
        print(e[0], e[1], w)
        if not is_directed:
            print(e[1], e[0], w)

    source_goal = random.sample(list(range(num_nodes)), 2)
    print(source_goal[0], source_goal[1])
