#include <gflags/gflags.h>
#include <unordered_map>
#include <unordered_set>

#include "LruClockCache/LruClockCache.h"
#include "basic_lru/lrucache.hpp"
#include "utils.h"

DEFINE_int64(storage_size, 10000, "");
DEFINE_int64(iterations, 10000, "");
DEFINE_int64(cache_size, 1000, "");

using key_type = std::string;
using value_type = std::string;

std::unordered_map<key_type, value_type> storage;
std::vector<key_type> keys;

void init_storage() {
  std::srand(std::time(0));
  for (int i = 0; i < FLAGS_storage_size; ++i) {
    std::string key = generateRandomString();
    std::string value = generateRandomString();
    storage[key] = value;
    keys.emplace_back(key);
  }
  printf("generated storage, size:%ld\n", FLAGS_storage_size);
}

void test_basic_lru() {
  /**Creates cache with maximum size of three. When the
     size in achieved every next element will replace the
     least recently used one */
  cache::lru_cache<std::string, std::string> cache(FLAGS_cache_size, storage);

  const std::string &from_cache = cache.get("two");
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  init_storage();

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
  printf("inited\n");
  for (int i = 0; i < FLAGS_iterations; ++i) {
    int offset = std::rand() % keys.size();
    // prinf("offset:%d, size:%lu\n", offset, keys.size());
    cache.get(keys[offset]);
  }
  printf("%ld iteration, %d miss\n", FLAGS_iterations, miss);

  return 0;
}