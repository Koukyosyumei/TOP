import argparse
import random

import networkx as nx
import osmnx as ox


def add_args(parser):
    parser.add_argument("-s", "--seed", default=42, type=int)

    args = parser.parse_args()
    return args


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parsed_args = add_args(parser)

    random.seed(parsed_args.seed)

    G = nx.Graph()
    G = ox.graph_from_point(
        center_point=(40.6156, -74.7706), network_type="drive", dist=3000
    )
    # ox.plot_graph(G, node_color='#87cefa', save=True, filepath='kobe.jpg')

    edges_info = ox.graph_to_gdfs(G, nodes=False, edges=True)
    node_ids = {n_id: i for i, n_id in enumerate(sorted(list(G.nodes)))}

    print(len(list(G.nodes)), len(list(G.edges)))
    for e in list(G.edges):
        print(
            node_ids[e[0]],
            node_ids[e[1]],
            edges_info.query(f"u=={e[0]} and v=={e[1]}")["length"].tolist()[0],
        )

    source_goal = random.sample(list(range(len(list(G.nodes)))), 2)
    print(source_goal[0], source_goal[1])
