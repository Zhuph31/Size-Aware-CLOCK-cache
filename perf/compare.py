from pprint import pprint
import matplotlib.pyplot as plt
import numpy as np


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


def custom_sort(can):
    # return (can.miss, can.tc, can.mem)
    return (can.mem, can.tc, can.miss)


def read_data(file):
    perf_data = dict()
    with open(file, "r") as file:
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

            if iterations not in perf_data.keys():
                perf_data[iterations] = dict()
            if cache_size not in perf_data[iterations].keys():
                perf_data[iterations][cache_size] = list()
            perf_data[iterations][cache_size].append(
                Candidate(
                    type,
                    alpha,
                    str(round(int(miss) / int(iterations) * 100, 2)),
                    tc,
                    mem,
                    rej,
                )
            )

    return perf_data


def sort_data():
    perf_data = read_data()
    for iterations, iter_val in perf_data.items():
        for cache_size, candidates in iter_val.items():
            sorted_candidates = sorted(candidates, key=custom_sort)
            print(
                "best candidate under {} {}, {}".format(
                    iterations, cache_size, sorted_candidates[0]
                )
            )


def plot_data(file):
    perf_data = read_data(file)
    for iterations, iter_val in perf_data.items():
        for cache_size, candidates in iter_val.items():
            miss = []
            mem = []
            rej = []
            tc = []

            # plot miss
            categories = []
            for candidate in candidates:
                categories.append(str(candidate.type))
                if categories[-1] == "2":
                    categories[-1] += "-" + str(candidate.alpha).rstrip("0")
                miss.append(float(candidate.miss))

            print(categories)
            print(miss)

            bar_width = 0.35

            bar1 = np.arange(len(categories))

            plt.bar(
                bar1,
                miss,
                color="blue",
                width=bar_width,
                label="Miss Percent",
                bottom=0,
            )
            plt.ylim(0, 100)

            plt.title("Miss Percent")
            plt.xlabel("Categories")
            plt.ylabel("Percentage")
            plt.xticks([r + bar_width / 2 for r in range(len(categories))], categories)

            plt.legend()
            plt.show()
            plt.savefig("perf1-miss")


if __name__ == "__main__":
    plot_data("perf1")
