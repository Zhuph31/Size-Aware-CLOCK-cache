#include <gflags/gflags.h>
#include <unordered_set>

#include "LruClockCache/LruClockCache.h"

DEFINE_int64(storage_size, 10000, "");
DEFINE_int64(iterations, 10000, "");
DEFINE_int64(cache_size, 1000, "");

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::unordered_map<int, int> storage;
  std::vector<int> keys;
  std::srand(std::time(0));
  for (int i = 0; i < FLAGS_storage_size; ++i) {
    int key = std::rand();
    int value = std::rand();
    storage[key] = value;
    keys.emplace_back(key);
  }
  printf("generated storage, size:%ld\n", FLAGS_storage_size);

  int miss = 0;
  auto read_miss = [&](int key) {
    ++miss;
    return storage.at(key);
  };
  auto write_miss = [&](int key, int value) {
    storage[key] = value;
    exit(-1);
  };
  LruClockCache<int, int> cache(FLAGS_cache_size, read_miss, write_miss);
  printf("inited\n");

  for (int i = 0; i < FLAGS_iterations; ++i) {
    int offset = std::rand() % keys.size();
    // prinf("offset:%d, size:%lu\n", offset, keys.size());
    cache.get(keys[offset]);
  }

  printf("%ld iteration, %d miss\n", FLAGS_iterations, miss);

  miss = 0;

  for (int i = 0; i < FLAGS_iterations; ++i) {
    int offset = std::rand() % keys.size();
    // prinf("offset:%d, size:%lu\n", offset, keys.size());
    // cache.get(keys[offset]);
    cache.get(keys[0]);
  }

  printf("%ld iteration, %d miss\n", FLAGS_iterations, miss);

  return 0;
}