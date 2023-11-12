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

DEFINE_int64(storage_size, 10000, "");
DEFINE_int64(iterations, 10000, "");
DEFINE_int64(cache_size, 1000, "");
DEFINE_int64(cache_mem_limit, 1073741824, "");
DEFINE_double(alpha, 0.2, "");
DEFINE_string(file, "hm_0.csv", "");
DEFINE_bool(real, false, "");

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
    printf("%ld,%d,%s,%ld\n", timestamp, static_cast<int>(type),
           cache_key.offset.c_str(), cache_key.length);
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

  rows.reserve(FLAGS_storage_size);

  long begin_ts = -1;
  long count = 0;

  while (count < FLAGS_storage_size && std::getline(file, line)) {
    ++count;

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

  count = 0;
  for (const Row &row : rows) {
    if (count < 10) {
      ++count;
      row.debug();
    }
    std::string value =
        FLAGS_real ? std::string(row.cache_key.length, 'a') : "true";
    storage[row.cache_key.key] = value;
  }

  printf("generated storage, size:%ld\n", rows.size());
}

std::pair<size_t, long long> test_basic_lru() {
  cache::lru_cache<key_type, value_type> cache(FLAGS_cache_size,
                                               FLAGS_cache_mem_limit, storage);

  TimeCost tc;
  for (const Row &row : rows) {
    cache.get(row.cache_key.key);
  }

  return {cache.get_miss(), tc.get_elapsed()};
}

std::pair<size_t, long long> test_clock_lru() {
  int miss = 0;
  auto read_miss = [&](key_type key) {
    ++miss;
    return storage.at(key);
  };
  auto write_miss = [&](key_type key, value_type value) {
    storage[key] = value;
    exit(-1);
  };
  LruClockCache<key_type, value_type> cache(
      FLAGS_cache_size, FLAGS_cache_mem_limit, read_miss, write_miss);

  TimeCost tc;
  for (const Row &row : rows) {
    cache.get(row.cache_key.key);
  }

  return {miss, tc.get_elapsed()};
}

std::pair<size_t, long long> test_my_clock() {
  int miss = 0;
  auto read_miss = [&](key_type key) {
    ++miss;
    return storage.at(key);
  };
  auto write_miss = [&](key_type key, value_type value) {
    storage[key] = value;
    exit(-1);
  };
  MyClockCache<key_type, value_type> cache(FLAGS_alpha, FLAGS_cache_size,
                                           FLAGS_cache_mem_limit, read_miss,
                                           write_miss);

  TimeCost tc;
  for (const Row &row : rows) {
    cache.get(row.cache_key.key);
  }
  return {miss, tc.get_elapsed()};
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  init_storage();

  size_t miss;
  long long cost;

  std::tie(miss, cost) = test_basic_lru();
  printf("basic lru, miss:%lu, cost:%lld\n", miss, cost);
  std::tie(miss, cost) = test_clock_lru();
  printf("clock lru, miss:%lu, cost:%lld\n", miss, cost);
  std::tie(miss, cost) = test_my_clock();
  printf("my lru, miss:%lu, cost:%lld\n", miss, cost);

  return 0;
}