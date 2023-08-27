import argparse
import random

import networkx as nx

goal = "G"
start = "S"
obstacle = ["@", "T"]
empty = "."


def is_valid_cell(grid, row, col):
    rows, cols = len(grid), len(grid[0])
    return 0 <= row < rows and 0 <= col < cols and grid[row][col] not in obstacle


def generate_grid_world(height, width, obstacle_prob):
    grid = [[empty for _ in range(width)] for _ in range(height)]

    start_x = random.randint(0, height - 1)
    start_y = random.randint(0, width - 1)

    while True:
        goal_x = random.randint(0, height - 1)
        goal_y = random.randint(0, width - 1)
        if start_x != goal_x or start_y != goal_y:
            break

    grid[start_x][start_y] = start
    grid[goal_x][goal_y] = goal

    num_obstacles = obstacle_prob * (height * width)
    for _ in range(int(num_obstacles)):
        while True:
            o_x = random.randint(0, height - 1)
            o_y = random.randint(0, width - 1)
            if grid[o_x][o_y] == empty:
                grid[o_x][o_y] = obstacle[0]
                break

    return grid


def grid_to_graph(grid):
    graph = nx.Graph()
    rows, cols = len(grid), len(grid[0])

    def node_id(row, col):
        return row * cols + col

    i = 0
    node_id_map = {}
    for row in range(rows):
        for col in range(cols):
            if grid[row][col] not in obstacle:
                node_id_map[node_id(row, col)] = i
                graph.add_node(i)
                i += 1

    for row in range(rows):
        for col in range(cols):
            if grid[row][col] in obstacle:
                continue

            if grid[row][col] == start:
                start_node = node_id_map[node_id(row, col)]
            elif grid[row][col] == goal:
                goal_node = node_id_map[node_id(row, col)]

            if is_valid_cell(grid, row - 1, col):  # Check the cell above
                graph.add_edge(
                    node_id_map[node_id(row, col)
                                ], node_id_map[node_id(row - 1, col)]
                )
            if is_valid_cell(grid, row + 1, col):  # Check the cell below
                graph.add_edge(
                    node_id_map[node_id(row, col)
                                ], node_id_map[node_id(row + 1, col)]
                )
            if is_valid_cell(grid, row, col - 1):  # Check the cell to the left
                graph.add_edge(
                    node_id_map[node_id(row, col)
                                ], node_id_map[node_id(row, col - 1)]
                )
            if is_valid_cell(grid, row, col + 1):  # Check the cell to the right
                graph.add_edge(
                    node_id_map[node_id(row, col)
                                ], node_id_map[node_id(row, col + 1)]
                )
    return graph, start_node, goal_node, node_id_map


def read_graph(path):
    with open(path, mode="r") as f:
        lines = f.readlines()
        height = int(lines[1].split(" ")[1])
        width = int(lines[2].split(" ")[1])
        maps = [list(lin) for lin in lines[4:]]
        maps[0] = maps[0][:-1]
        while True:
            start_x = random.randint(0, width - 1)
            start_y = random.randint(0, height - 1)
            goal_x = random.randint(0, width - 1)
            goal_y = random.randint(0, height - 1)
            if maps[start_y][start_x] == empty:
                maps[start_y][start_x] = start
            else:
                continue
            if maps[goal_y][goal_x] == empty:
                maps[goal_y][goal_x] = goal
                break
    return maps


def print_out_networkx_graph(G, transit_candidates, source_goal=None):
    num_nodes = len(G.nodes)
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

    if source_goal is None:
        source_goal = random.sample(list(range(num_nodes)), 2)
    print(source_goal[0], source_goal[1])

    print(len(transit_candidates))
    for t in transit_candidates:
        print(t)

    pass


def add_args(parser):
    parser.add_argument("-t", "--gtype", default="grid", type=str)
    parser.add_argument("-g", "--gridfile", type=str)
    parser.add_argument(
        "-n",
        "--graph_size",
        default=10,
        type=int,
    )
    parser.add_argument(
        "-e",
        "--edges_fraction",
        default=0.3,
        type=float,
    )
    parser.add_argument("-c", "--num_transit_candidates", default=-1, type=int)
    parser.add_argument(
        "-o",
        "--obstacle_frac",
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

    graph_size = parsed_args.graph_size
    edges_fraction = parsed_args.edges_fraction
    seed = parsed_args.seed
    is_directed = parsed_args.is_directed

    random.seed(seed)

    G = nx.Graph()
    source_and_goal = None
    node_id_map = None

    if parsed_args.gtype == "gnp":
        G = nx.random_graphs.fast_gnp_random_graph(
            graph_size, edges_fraction, seed, directed=is_directed
        )
    elif parsed_args.gtype == "internet":
        G = nx.random_internet_as_graph(graph_size)
    elif parsed_args.gtype == "sudoku":
        G = nx.sudoku_graph(graph_size)
    elif parsed_args.gtype == "grid":
        grid = generate_grid_world(
            graph_size, graph_size, parsed_args.obstacle_frac)
        G, source, goal, node_id_map = grid_to_graph(grid)
        source_and_goal = (source, goal)
    elif parsed_args.gtype == "fgrid":
        grid = read_graph(parsed_args.gridfile)
        G, source, goal, node_id_map = grid_to_graph(grid)
        source_and_goal = (source, goal)

    if parsed_args.num_transit_candidates == -1:
        transit_candidates = list(set(G.nodes) - set(source_and_goal))
    else:
        transit_candidates = random.sample(
            list(G.nodes), parsed_args.num_transit_candidates
        )
    print_out_networkx_graph(G, transit_candidates, source_and_goal)
    for k, v in node_id_map.items():
        print(k, v)
