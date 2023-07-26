import argparse
import pandas as pd
import glob
import re


def add_args(parser):
    parser.add_argument("-d", "--dir", default="output/*", type=str)
    parser.add_argument(
        "-o", "--output_file_path", default="report_summary.csv", type=str
    )
    parser.add_argument(
        "-t", "--output_time_file_path", default="report_time.csv", type=str
    )
    args = parser.parse_args()
    return args


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parsed_args = add_args(parser)

    rows = []
    df_times_logs = []
    for path in glob.glob(parsed_args.dir):
        matches = re.findall(r"=(.*?)_", path)
        gtype = "null"
        n = -1
        ep = -1
        s = -1
        if len(matches) >= 4:
            gtype = matches[0]
            n = matches[1]
            ep = matches[2]
            s = matches[3]

        with open(path, mode="r") as f:
            lines = f.readlines()
        lines = [lin.replace("\n", "") for lin in lines]

        if len(lines) < 10:
            continue

        k = lines[1].split("=")[1]
        el = lines[2].split("=")[1]
        v = lines[3].split("=")[1]
        h = lines[4].split("=")[1]
        j = lines[5].split("=")[1]
        c = lines[6].split("=")[1]
        u = lines[7].split("=")[1]
        p = lines[8].split("=")[1]
        r = lines[9].split("=")[1]

        times_log = []
        for i, lin in enumerate(lines):
            if "[ms]" in lin:
                infos = lin.split(", ")
                parsed_infos = [float(i.split(" ")[0]) for i in infos]
                times_log.append([path] + parsed_infos)
            if "Performance Summary of DFBB" in lin:
                break

        if len(lines) == i + 1:
            continue

        if len(times_log) > 0:
            df_time_single = pd.DataFrame(times_log)
            df_time_single.columns = [
                "path",
                "time",
                "NumberOfAnonymizedPaths",
                "AverageCost",
                "NumberOfPartitions",
                "NumberOfValidPartitions",
                "NumberofSkipped",
                "NumberOfDuplicated",
                "NumberOfNodes",
            ]
            df_times_logs.append(df_time_single)

        num_evaluated_partitions = lines[i + 1].split(": ")[1]
        num_valid_partitions = lines[i + 2].split(": ")[1]
        num_skipped_partitions = lines[i + 3].split(": ")[1]
        num_expanded_nodes = lines[i + 4].split(": ")[1]
        num_evaluated_partitions_till_first = lines[i + 5].split(": ")[1]
        num_expanded_nodes_till_first = lines[i + 6].split(": ")[1]
        num_duplicated_partitions = lines[i + 7].split(": ")[1]
        num_anonymized_paths = lines[i + 8].split(": ")[1]
        avg_cost_anonymized_paths = lines[i + 9].split(": ")[1]
        total_time = lines[i + 10].split(": ")[1].split(" [ms]")[0]
        lowerbound_cost = lines[i + 11].split(": ")[1]

        rows.append(
            [
                path,
                gtype,
                n,
                ep,
                s,
                k,
                el,
                v,
                h,
                j,
                c,
                u,
                p,
                r,
                num_evaluated_partitions,
                num_valid_partitions,
                num_skipped_partitions,
                num_expanded_nodes,
                num_anonymized_paths,
                num_evaluated_partitions_till_first,
                num_expanded_nodes_till_first,
                avg_cost_anonymized_paths,
                total_time,
                lowerbound_cost,
                num_duplicated_partitions,
            ]
        )

    df = pd.DataFrame(rows)
    df.columns = [
        "path",
        "graph_type",
        "num_nodes",
        "probability_of_edge",
        "seed",
        "k",
        "el",
        "visibility_func",
        "heuristic_func",
        "order_of_j",
        "complete_search",
        "upperbound_pruning",
        "partition_type",
        "radius",
        "num_evaluated_partitions",
        "num_valid_partitions",
        "num_skipped_partitions",
        "num_expanded_nodes",
        "num_anonymized_paths",
        "num_evaluated_partitions_till_first",
        "num_expanded_nodes_till_first",
        "avg_cost_anonymized_paths",
        "total_time",
        "lowebound_cost",
        "num_duplicated_partitions",
    ]
    df.to_csv(parsed_args.output_file_path, index=False)

    df_time = pd.concat(df_times_logs)
    df_time.to_csv(parsed_args.output_time_file_path, index=False)
