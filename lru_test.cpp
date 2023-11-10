#include <gflags/gflags.h>
#include <unordered_map>
#include <unordered_set>

#include "LruClockCache/LruClockCache.h"
#include "MyClock/MyClock.h"
#include "basic_lru/lrucache.hpp"
#include "utils.h"

DEFINE_int64(storage_size, 10000, "");
DEFINE_int64(iterations, 10000, "");
DEFINE_int64(cache_size, 1000, "");
DEFINE_double(alpha, 0.2, "");

using key_type = std::string;
using value_type = std::string;

std::unordered_map<key_type, value_type> storage;
std::vector<key_type> test_keys;

void init_storage() {
  std::srand(std::time(0));
  for (int i = 0; i < FLAGS_storage_size; ++i) {
    std::string key = generateRandomString();
    std::string value = generateRandomString();
    storage[key] = value;
    test_keys.emplace_back(key);
  }
  printf("generated storage, size:%ld\n", FLAGS_storage_size);
}

std::pair<size_t, long long> test_basic_lru() {
  cache::lru_cache<std::string, std::string> cache(FLAGS_cache_size, storage);
  TimeCost tc;

  for (const key_type &each : test_keys) {
    cache.get(each);
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
  LruClockCache<key_type, value_type> cache(FLAGS_cache_size, read_miss,
                                            write_miss);

  TimeCost tc;
  for (const key_type &each : test_keys) {
    cache.get(each);
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
                                           read_miss, write_miss);

  TimeCost tc;
  for (const key_type &each : test_keys) {
    cache.get(each);
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