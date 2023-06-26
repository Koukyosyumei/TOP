import argparse
import random
import os


def add_args(parser):
    parser.add_argument("-f", "--file", type=str)
    parser.add_argument("-d", "--output_dir", type=str)
    parser.add_argument("-s", "--seed", default=42, type=int)
    parser.add_argument("-n", "--num_outputs", default=1, type=int)
    args = parser.parse_args()
    return args


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parsed_args = add_args(parser)

    random.seed(parsed_args.seed)

    with open(parsed_args.file, mode="r") as f:
        lines = f.readlines()

    base_name = parsed_args.file.split("/")[-1]
    N, _ = tuple(map(int, lines[0][:-1].split(" ")))

    for i in range(parsed_args.num_outputs):
        source_goal = random.sample(list(range((N))), 2)
        with open(
            os.path.join(parsed_args.output_dir, base_name + "-" + str(i) + ".in"), "w"
        ) as file:
            for line in lines[:-1]:
                file.write(line)
            file.write(f"{source_goal[0]} {source_goal[1]}")
