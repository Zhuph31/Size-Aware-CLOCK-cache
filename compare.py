from pprint import pprint


file_path = "perf/perf2"

rank = dict()


class Candidate:
    def __init__(self, type, alpha, miss, tc, mem, rej):
        self.type = type
        self.alpha = alpha
        self.miss = miss
        self.tc = tc
        self.mem = mem
        self.rej = rej

    def __str__(self):
        return f"type:{self.type} alpha:{self.alpha} miss:{self.miss} tc:{self.tc} mem:{self.mem} rej:{self.rej}"


with open(file_path, "r") as file:
    lines = file.readlines()

    for line in lines:
        res = line.split()
        type = res[0]
        iterations = res[1]
        cache_size = res[2]
        alpha = res[3]
        miss = res[4]
        tc = res[5]
        mem = res[6]
        rej = res[7]

        if iterations not in rank.keys():
            rank[iterations] = dict()
        if cache_size not in rank[iterations].keys():
            rank[iterations][cache_size] = list()
        rank[iterations][cache_size].append(Candidate(type, alpha, miss, tc, mem, rej))


def custom_sort(can):
    # return (can.miss, can.tc, can.mem)
    return (can.mem, can.tc, can.miss)


for iterations, iter_val in rank.items():
    for cache_size, candidates in iter_val.items():
        sorted_candidates = sorted(candidates, key=custom_sort)
        print(
            "best candidate under {} {}, {}".format(
                iterations, cache_size, sorted_candidates[0]
            )
        )
