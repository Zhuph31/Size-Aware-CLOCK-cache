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

void init_storage() {
  std::ifstream file(FLAGS_file);
  if (!file.is_open()) {
    exit(-2);
  }
  std::string line;

  long begin_ts = -1;

  while (std::getline(file, line)) {

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
                          offset, std::stol(length)));
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
};

Metrics test_basic_lru(uint64_t iterations, uint64_t cache_size, double alpha) {
  cache::lru_cache<key_type, value_type> cache(cache_size, storage);
  size_t count = 0, mem_consume = 0;

  TimeCost tc;
  for (int i = 0; i < iterations; ++i) {
    const Row &row = rows[i % rows.size()];
    auto &val = cache.get(row.cache_key.key);
    // printf("key:%lu, val:%lu\n", row.cache_key.key.size(), val.size());
    mem_consume = (mem_consume * count + cache.get_mem_consume()) / (count + 1);
    ++count;
  }

  return {cache.get_miss(), tc.get_elapsed(), mem_consume};
}

Metrics test_clock_lru(bool my, uint64_t iterations, uint64_t cache_size,
                       double alpha) {
  size_t miss = 0;
  size_t count = 0, mem_consume = 0;
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
      mem_consume =
          (mem_consume * count + cache.get_mem_consume()) / (count + 1);
      ++count;
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
      mem_consume =
          (mem_consume * count + cache.get_mem_consume()) / (count + 1);
      ++count;
    }
    return {miss, tc.get_elapsed(), mem_consume, cache.get_rejects()};
  }
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  init_storage();

  std::vector<uint64_t> iteration_options = {1000000, 10000000, 100000000};
  std::vector<uint64_t> cache_size_options = {1000, 10000, 100000};
  std::vector<double> alpha_options = {0.2, 0.4, 0.6, 0.8};

  for (uint64_t iteratinos : iteration_options) {
    for (uint64_t cache_size : cache_size_options) {
      test_basic_lru(iteratinos, cache_size, 0).print();
      test_clock_lru(false, iteratinos, cache_size, 0).print();
      for (uint64_t alpha : alpha_options) {
        test_clock_lru(true, iteratinos, cache_size, alpha).print();
      }
    }
  }

  return 0;
}