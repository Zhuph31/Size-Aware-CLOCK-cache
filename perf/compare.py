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
            alpha = float(res[3])
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
    print(len(candidates))

    if type == "miss":
        label = "Miss Percent"
        plt.ylim(20, 70)
        plt.ylabel("Percentage")
    elif type == "mem":
        label = "Memory Usage"
        plt.ylabel("Bytes")
    elif type == "tc":
        label = "Time Cost"
    elif type == "rej":
        label = "Rejections"

    for candidate in candidates:
        categories.append(candidate.alpha)
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
    # for i, value in enumerate(val):
    #     plt.text(i, value + 0.5, str(value), ha="center", va="bottom")
    print(plt.ylim())

    plt.title(label)
    # plt.xlabel("Categories")
    plt.xticks([r for r in range(len(categories))], categories)
    plt.xticks(rotation=45)

    # plt.legend()
    # plt.show()
    plt.savefig("alpha_pic/{}".format(type))
    plt.clf()


def plot_data(file):
    perf_data = read_data(file)
    for iterations, iter_val in perf_data.items():
        for cache_size, candidates in iter_val.items():
            plot_val(iterations, cache_size, candidates, "miss")
            plot_val(iterations, cache_size, candidates, "mem")
            plot_val(iterations, cache_size, candidates, "tc")
            plot_val(iterations, cache_size, candidates, "rej")


def plot_multiple_curves(file):
    with open(file, "r") as file:
        data_lines = file.readlines()

    print(len(data_lines))

    data1 = list(map(float, data_lines[0].split()))
    data2 = list(map(float, data_lines[1].split()))
    data3 = list(map(float, data_lines[2].split()))
    print(len(data1))
    print(len(data2))
    print(len(data3))

    x_values = list(range(len(data1)))
    print(len(x_values))

    plt.plot(x_values, data1, label="LRU")
    plt.plot(x_values, data2, label="CLOCK")
    plt.plot(x_values, data3, label="SA-CLOCK")

    plt.title("Miss Ratio Over Time")
    # plt.xlabel("Time")
    # plt.xticks([])
    plt.ylabel("Percentage")

    plt.legend()
    plt.savefig("fake_miss1")


def plot_curve(file):
    with open(file, "r") as file:
        data_lines = file.readlines()

    print(len(data_lines))

    data1 = list(map(float, data_lines[0].split()))

    print(len(data1))

    x_values = list(range(len(data1)))
    print(len(x_values))

    plt.plot(x_values, data1)

    plt.title("Input Size")
    plt.ylabel("Object Size (Bytes)")
    # plt.xticks([])

    # plt.legend()
    plt.savefig("fake_input1")


if __name__ == "__main__":
    # plot_data("adapt2")
    # plot_data("perf2")
    # plot_data("perf3")
    # plot_data("perf4")
    plot_data("alpha")
    # plot_mem_record("mem_records")
    # plot_multiple_curves("miss_records")
    # plot_multiple_curves("fake1_miss")
    # plot_curve("fake1")
    # plot_multiple_curves("")
