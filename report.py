import argparse
import pandas as pd
import glob
import re


def add_args(parser):
    parser.add_argument("-d", "--dir", default="output", type=str)
    parser.add_argument("-o", "--output_file_path",
                        default="report.csv", type=str)
    args = parser.parse_args()
    return args


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parsed_args = add_args(parser)

    rows = []
    for path in glob.glob(parsed_args.dir):
        matches = re.findall(r'=(.*?)_', path)
        n = matches[0]
        p = matches[1]
        s = matches[2]
        with open(path, mode="r") as f:
            lines = f.readlines()
        lines = [lin.replace("\n", "") for lin in lines]
        k = lines[1].split("=")[1]
        el = lines[2].split("=")[1]
        v = lines[3].split("=")[1]
        h = lines[4].split("=")[1]
        j = lines[5].split("=")[1]
        c = lines[6].split("=")[1]
        u = lines[7].split("=")[1]

        for i, lin in enumerate(lines):
            if "Performance Summary of DFBB" in lin:
                break

        if (len(lines) == i + 1):
            continue

        num_evaluated_partitions = int(lines[i+1].split(": ")[1])
        num_valid_partitions = int(lines[i+2].split(": ")[1])
        num_skipped_partitions = int(lines[i+3].split(": ")[1])
        num_expanded_nodes = int(lines[i+4].split(": ")[1])
        num_anonymized_paths = int(lines[i+5].split(": ")[1])
        avg_cost_anonymized_paths = float(lines[i+6].split(": ")[1])

        rows.append([n, p, s, k, el, v, h, j, c, u,
                     num_evaluated_partitions, num_valid_partitions,
                     num_skipped_partitions, num_expanded_nodes,
                     num_anonymized_paths, avg_cost_anonymized_paths])

    df = pd.DataFrame(rows)
    columns = ["num_nodes", "probability_of_edge", "seed",
               "k", "el", "visibility_func", "heuristic_func", "order_of_j",
               "complete_search", "upperbound_pruning",
               "num_evaluated_partitions", "num_valid_partitions",
               "num_skipped_partitions", "num_expanded_nodes",
               "num_anonymized_paths", "avg_cost_anonymized_paths"]
    df.columns = columns
    df.to_csv(parsed_args.output_file_path, index=False)
