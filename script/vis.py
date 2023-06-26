import networkx as nx


def parse_input(input_path):
    with open(input_path, mode="r") as f:
        lines = f.readlines()

    tuples = [tuple(map(int, line[:-1].split(" "))) for line in lines]
    N, E = tuples[0]
    s, g = tuples[-1]
    edges = tuples[1:-1]
    edges_lists = [(e[0], e[1], {"weight": e[2]}) for e in edges]

    return s, g, edges_lists


def parse_output(output_path):
    with open(output_path, mode="r") as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if "Displaying" in line:
            break

    ps = []
    for line in lines[i + 1 :]:
        if "Partition" in line:
            ps.append([])
        else:
            ps[-1].append(list(map(int, line[:-1].split(" ")[:-1])))

    return ps


def extract_partition(ps, p_id):
    partition = ps[p_id]
    elements = partition[0]
    path = [e[0] for e in partition[1:]]
    path_edges = list(zip(path, path[1:]))

    return elements, path, path_edges


def vis_graph(G, s, g, path, elements, path_edges):
    pos = nx.spring_layout(G, seed=0)
    nx.draw(G, pos, node_color="k")

    nx.draw_networkx_nodes(G, pos, nodelist=[s], node_color="b")
    nx.draw_networkx_labels(G, pos, labels={s: "S", g: "G"}, font_color="white")
    nx.draw_networkx_labels(
        G,
        pos,
        labels={p: i for i, p in enumerate(path[1:-1], start=1)},
        font_color="white",
    )
    nx.draw_networkx_nodes(G, pos, nodelist=[g], node_color="g")
    nx.draw_networkx_nodes(G, pos, nodelist=elements, node_color="r")
    nx.draw_networkx_edges(G, pos, edgelist=path_edges, edge_color="r", width=2)


if __name__ == "__main__":
    s, g, edges_list = parse_input("12.in")
    ps = parse_output("12.out")

    G = nx.Graph(edges_list)

    elements, path, path_edges = extract_partition(ps, 0)
    vis_graph(G, s, g, path, elements, path_edges)
