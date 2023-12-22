# TOP

Code for "On the Transit Obfuscation Problem"

## CLI TOol

- Build

```
./script/build.sh
```

- Usage

```
```

- Options

```
-k: k of (k, el, m)-Anonymity (default: 2).
-l: el of (k, el, m)-Anonymity (default: 0).
-m: m of (k, el, m)-Anonymity (default: -1, which is equivalent to infinity).
-v: Visibility function type (options: identity, onestep, radius) (default: identity).
-r: Radius for the radius-based visibility function (default: 1).
-p: Partitioning type (options: merge, df, df+, random, clustering, naive, wrp) (default: merge).
-h: Heuristic function type (options: blind, tunnel, tunnel+) (default: blind).
-j: Order type for partition merging (options: random) (default: random).
-b: Verbose level (default: 1000).
-t: Timeout for the search process (default: infinity).
-f: Path to the log file (default: log.out).
-c: Enable complete search.
-u: Use upper-bound cost.
-a: Print cover path details.
```

- Input Format

The tool expects input in the following format from standard input:

```
N E           // Number of nodes (N) and edges (E)
a b c         // Edge information (a and b are nodes, c is cost)
source goal   // Source and goal nodes
num_transit_candidates   // Number of transit candidates
transit_candidates       // List of transit candidates
```

- Output

The tool outputs details about the setup and, if successful, information about the optimal partition, including the partition type, the number of nodes expanded, and the elements within each partition.


