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
            if float(alpha) > 0.2:
                continue
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


def plot_val(iterations, cache_size, candidates, type):
    print(type)
    val = []
    categories = []
    label = ""

    if type == "miss":
        label = "Miss Percent"
        plt.ylim(20, 60)
        plt.ylabel("Percentage")
    elif type == "mem":
        label = "Memory Usage"
        plt.ylabel("Bytes")
    elif type == "tc":
        label = "Time Cost"
    elif type == "rej":
        label = "Rejections"

    for candidate in candidates:
        if candidate.type == "0":
            categories.append("LRU")
        elif candidate.type == "1":
            categories.append("CLOCK")
        else:
            categories.append("SA-CLOCK")
        if type == "miss":
            val.append(float(candidate.miss))
        elif type == "mem":
            val.append(int(candidate.mem))
        elif type == "tc":
            val.append(int(candidate.tc))
        elif type == "rej":
            val.append(int(candidate.rej))

    print(categories)
    print(val)

    bar_width = 0.35

    bar1 = np.arange(len(categories))

    plt.bar(bar1, val, color="blue", width=bar_width, bottom=0, align="center")
    for i, value in enumerate(val):
        plt.text(i, value + 0.5, str(value), ha="center", va="bottom")
    print(plt.ylim())

    plt.title(label)
    # plt.xlabel("Categories")
    plt.xticks([r for r in range(len(categories))], categories)

    # plt.legend()
    # plt.show()
    plt.savefig("{}/{}-{}-{}".format(type, iterations, cache_size, type))
    plt.clf()


def plot_data(file):
    perf_data = read_data(file)
    for iterations, iter_val in perf_data.items():
        for cache_size, candidates in iter_val.items():
            plot_val(iterations, cache_size, candidates, "miss")
            plot_val(iterations, cache_size, candidates, "mem")
            plot_val(iterations, cache_size, candidates, "tc")
            plot_val(iterations, cache_size, candidates, "rej")


def plot_mem_record(file):
    with open(file, "r") as file:
        data_lines = file.readlines()

    print(len(data_lines))

    data1 = list(map(int, data_lines[0].split()))
    data2 = list(map(int, data_lines[1].split()))
    data3 = list(map(int, data_lines[2].split()))
    print(len(data1))
    print(len(data2))
    print(len(data3))

    x_values = list(range(len(data1)))
    print(len(x_values))

    plt.plot(x_values, data1, label="LRU")
    plt.plot(x_values, data2, label="CLOCK")
    plt.plot(x_values, data3, label="SA-CLOCK")

    plt.title("Memory Usage Over Time")
    # plt.xlabel("Time")
    plt.xticks([])
    plt.ylabel("Memory Usage (bytes)")

    plt.legend()
    plt.savefig("mem_records")


def plot_curve(file):
    with open(file, "r") as file:
        data_lines = file.readlines()

    print(len(data_lines))

    data1 = list(map(int, data_lines[0].split()))
    # data1 = data1[::1000]

    print(len(data1))

    x_values = list(range(len(data1)))
    print(len(x_values))

    plt.plot(x_values, data1)

    plt.title("Input SIze")
    plt.ylabel("Object Size (bytes)")
    # plt.xticks([])

    # plt.legend()
    plt.savefig("fake_input2")


if __name__ == "__main__":
    # plot_data("perf1")
    # plot_data("perf2")
    # plot_data("perf3")
    # plot_data("perf4")
    # plot_data("perf.1")
    # plot_mem_record("mem_records")
    # plot_input_size("input_size")
    plot_curve("fake2")
