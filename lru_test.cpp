#include <fstream>
#include <gflags/gflags.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "LruClockCache/LruClockCache.h"
#include "MyClock/MyClock.h"
#include "basic_lru/lrucache.hpp"
#include "utils.h"

// DEFINE_int64(storage_size, 10000, "");
// DEFINE_int64(iterations, 1000000, "");
// DEFINE_int64(cache_size, 1000, "");
// DEFINE_double(alpha, 0.2, "");
DEFINE_string(file, "hm_0.csv", "");
DEFINE_uint64(storage_size, 1000000, "");
DEFINE_int32(record_mem, 0, "");
DEFINE_int32(record_miss, 0, "");
DEFINE_int32(record_threshold, 0, "");
DEFINE_int32(record_rej, 0, "");
DEFINE_bool(record_input, false, "");
DEFINE_int32(fake_storage, 0, "");

using key_type = std::string;
using value_type = std::string;

enum class VisitType {
  READ = 0,
  WRITE = 1,
};

struct CacheKey {
  std::string offset;
  long length;
  std::string key;

  CacheKey() = delete;
  CacheKey(const std::string &o, long l)
      : offset(o), length(l), key(offset + "_" + std::to_string(length)) {}
};

struct Row {
  long timestamp; // 100ns
  VisitType type;
  CacheKey cache_key;

  Row() = delete;
  Row(long ts, VisitType t, const std::string &o, long l)
      : timestamp(ts), type(t), cache_key(o, l) {}
  void debug() const {
    // printf("%ld,%d,%s,%ld\n", timestamp, static_cast<int>(type),
    //        cache_key.offset.c_str(), cache_key.length);
  }
};

std::vector<Row> rows;
std::unordered_map<std::string /* offset */, std::string> storage;
std::vector<size_t> mem_records;
std::vector<double> lru_miss_rate_records;
std::vector<double> clock_miss_rate_records;
std::vector<double> sa_clock_miss_rate_records;
std::vector<size_t> threshold_records;
std::vector<size_t> rej_records;

long gen_fake_len(size_t real_len) {
  static bool small = true;
  static long small_limit = 100000, large_floor = 500000;
  static long count = 0;
  ++count;
  if (FLAGS_fake_storage == 2) {
    if (count % (FLAGS_storage_size / 4) == 0) {
      small = !small;
    }
  } else {
    if (count % (FLAGS_storage_size / 10) == 0 ||
        count % (FLAGS_storage_size / 10 + 1000) == 0) {
      small = !small;
    }
  }

  long fake_len = 0;
  if (small) {
    fake_len = real_len % small_limit;
  } else {
    fake_len = real_len % 10000 + large_floor;
  }
  // printf("gen fake len:%ld vs %ld\n", real_len, fake_len);
  return fake_len;
}

void init_storage() {
  std::ifstream file(FLAGS_file);
  if (!file.is_open()) {
    exit(-2);
  }
  std::string line;

  long begin_ts = -1;
  int count = 0;

  while (std::getline(file, line) && ++count < FLAGS_storage_size) {
    std::istringstream iss(line);
    std::string value;
    std::vector<std::string> values;
    while (std::getline(iss, value, ',')) {
      values.push_back(value);
    }

    const std::string &ts = values[0], type = values[3], offset = values[4],
                      length = values[5];
    long relative_ts = 0;

    if (begin_ts != -1) {
      relative_ts = std::stol(ts) - begin_ts;
    } else {
      begin_ts = std::stol(ts);
    }
    rows.emplace_back(Row(relative_ts,
                          type == "Read" ? VisitType::READ : VisitType::WRITE,
                          offset,
                          FLAGS_fake_storage ? gen_fake_len(std::stol(length))
                                             : std::stol(length)));
    // printf("row len:%ld\n", rows.back().cache_key.length);
  }

  for (const Row &row : rows) {
    storage[row.cache_key.key] = std::string(row.cache_key.length, 'a');
  }

  printf("storage inited\n");
}

struct Metrics {
  size_t miss;
  size_t tc;
  size_t mem_consume;
  size_t rejects;
  Metrics() = delete;
  Metrics(size_t m, size_t t, size_t mc, size_t r = 0)
      : miss(m), tc(t), mem_consume(mc), rejects(r) {}
  void print() const {
    // printf("miss:%lu, tc:%lu, mem consume:%lu, rejects:%lu\n", miss, tc,
    //        mem_consume, rejects);
    printf("%lu %lu %lu %lu ", miss, tc, mem_consume, rejects);
  }

  std::string to_string() const {
    return string_printf("%lu %lu %lu %lu ", miss, tc, mem_consume, rejects);
  }
};

Metrics test_basic_lru(uint64_t iterations, uint64_t cache_size, double alpha) {
  cache::lru_cache<key_type, value_type> cache(cache_size, storage);
  size_t mem_consume = 0;

  TimeCost tc;
  for (int i = 0; i < iterations; ++i) {
    const Row &row = rows[i % rows.size()];
    auto &val = cache.get(row.cache_key.key);
    // printf("key:%lu, val:%lu\n", row.cache_key.key.size(), val.size());
    size_t cur_mem_consum = cache.get_mem_consume();
    mem_consume = (mem_consume * i + cur_mem_consum) / (i + 1);
    if (FLAGS_record_mem > 0 && i % FLAGS_record_mem == 0) {
      mem_records.emplace_back(cur_mem_consum);
    }
    if (FLAGS_record_miss > 0 && i % FLAGS_record_miss == 0 && i != 0) {
      lru_miss_rate_records.emplace_back(static_cast<double>(cache.get_miss()) /
                                         static_cast<double>(i));
    }
  }

  return {cache.get_miss(), tc.get_elapsed(), mem_consume};
}

Metrics test_clock_lru(bool my, uint64_t iterations, uint64_t cache_size,
                       double alpha) {
  size_t miss = 0;
  size_t mem_consume = 0;
  auto read_miss = [&](key_type key) {
    ++miss;
    return storage.at(key);
  };
  auto write_miss = [&](key_type key, value_type value) {
    storage[key] = value;
    exit(-1);
  };

  if (!my) {
    LruClockCache<key_type, value_type> cache(cache_size, read_miss,
                                              write_miss);

    TimeCost tc;
    for (int i = 0; i < iterations; ++i) {
      const Row &row = rows[i % rows.size()];
      auto &val = cache.get(row.cache_key.key);
      // printf("key:%lu, val:%lu\n", row.cache_key.key.size(), val.size());
      size_t cur_mem_consum = cache.get_mem_consume();
      mem_consume = (mem_consume * i + cur_mem_consum) / (i + 1);
      if (FLAGS_record_mem > 0 && i % FLAGS_record_mem == 0) {
        mem_records.emplace_back(cur_mem_consum);
      }
      if (FLAGS_record_miss > 0 && i % FLAGS_record_miss == 0 && i != 0) {
        clock_miss_rate_records.emplace_back(double(miss) / double(i));
      }
    }

    return {miss, tc.get_elapsed(), mem_consume};
  } else {
    MyClockCache<key_type, value_type> cache(alpha, cache_size, read_miss,
                                             write_miss);

    TimeCost tc;
    for (int i = 0; i < iterations; ++i) {
      const Row &row = rows[i % rows.size()];
      auto &val = cache.get(row.cache_key.key);
      // printf("key:%lu, val:%lu\n", row.cache_key.key.size(), val.size());
      size_t cur_mem_consum = cache.get_mem_consume();
      // printf("get cur mem :%ld", cur_mem_consum);
      mem_consume = (mem_consume * i + cur_mem_consum) / (i + 1);
      if (FLAGS_record_mem > 0 && i % FLAGS_record_mem == 0) {
        mem_records.emplace_back(cur_mem_consum);
      }
      if (FLAGS_record_threshold > 0 && i % FLAGS_record_threshold == 0) {
        threshold_records.emplace_back(cache.get_threshold());
      }
      if (FLAGS_record_rej > 0 && i % FLAGS_record_rej == 0) {
        rej_records.emplace_back(cache.get_rej_size());
      }
      if (FLAGS_record_miss > 0 && i % FLAGS_record_miss == 0 && i != 0) {
        sa_clock_miss_rate_records.emplace_back(double(miss) / double(i));
      }
    }
    return {miss, tc.get_elapsed(), mem_consume, cache.get_rejects()};
  }
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  init_storage();

  if (FLAGS_record_input) {
    std::ofstream of("input_size");
    for (int i = 0; i < FLAGS_storage_size; ++i) {
      if (i % 100 != 0) {
        continue;
      }
      const Row &row = rows[i % rows.size()];
      auto &val = storage.at(row.cache_key.key);
      size_t val_size = val.size();
      of << val_size << " ";
    }
    of.close();
    return 0;
  }

  std::vector<uint64_t> iteration_options = {FLAGS_storage_size};
  std::vector<uint64_t> cache_size_options = {1000};
  std::vector<double> alpha_options = {0.1};
  std::vector<size_t> lru_mem_records, clock_mem_records, sa_clock_mem_records;

  for (uint64_t iterations : iteration_options) {
    for (uint64_t cache_size : cache_size_options) {
      printf("0 %lu %lu -1 %s\n", iterations, cache_size,
             test_basic_lru(iterations, cache_size, 0).to_string().c_str());
      lru_mem_records = mem_records;
      mem_records.clear();

      printf(
          "1 %lu %lu -1 %s\n", iterations, cache_size,
          test_clock_lru(false, iterations, cache_size, 0).to_string().c_str());
      clock_mem_records = mem_records;
      mem_records.clear();

      for (double alpha : alpha_options) {
        printf("2 %lu %lu %lf %s\n", iterations, cache_size, alpha,
               test_clock_lru(true, iterations, cache_size, alpha)
                   .to_string()
                   .c_str());
        sa_clock_mem_records = mem_records;
        mem_records.clear();
      }
    }
  }

  if (FLAGS_record_miss > 0) {
    std::ofstream outputFile("miss_records");
    for (const double each : lru_miss_rate_records) {
      outputFile << each << " ";
    }
    outputFile << "\n";
    for (const double each : clock_miss_rate_records) {
      outputFile << each << " ";
    }
    outputFile << "\n";
    for (const double each : sa_clock_miss_rate_records) {
      outputFile << each << " ";
    }
    outputFile << "\n";
    outputFile.close();
  }

  // {
  //   std::ofstream outputFile("mem_records");
  //   for (const size_t each : lru_mem_records) {
  //     outputFile << each << " ";
  //   }
  //   outputFile << "\n";
  //   for (const size_t each : clock_mem_records) {
  //     outputFile << each << " ";
  //   }
  //   outputFile << "\n";
  //   for (const size_t each : sa_clock_mem_records) {
  //     outputFile << each << " ";
  //   }
  //   outputFile << "\n";
  //   outputFile.close();
  // }

  // {
  //   std::ofstream outputFile("rej_records");
  //   for (const size_t each : rej_records) {
  //     outputFile << each << " ";
  //   }
  //   outputFile << "\n";
  //   outputFile.close();
  // }

  // {
  //   std::ofstream outputFile("threshod_records");
  //   for (const size_t each : threshold_records) {
  //     outputFile << each << " ";
  //   }
  //   outputFile << "\n";
  //   outputFile.close();
  // }

  return 0;
}