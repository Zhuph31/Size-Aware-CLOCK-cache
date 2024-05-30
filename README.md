# Source code for paper Achieving Low Memory Usage With Size-Aware Recency Based Cache

A CLOCK cache that takes objects' sizes into consideration.

## Implementations
- The LRU implementation can be found under basic_lru folder. It is modified based on https://github.com/lamerman/cpp-lru-cache/blob/master/include/lrucache.hpp. The code is modified to be able to retrieve from in-memory storage for the missing value when cache miss happens. Also it is able to record information throughout the experiments.

- The CLOCK implementation can be found under LruClockCache folder. It is modified based on https://github.com/tugrul512bit/LruClockCache/blob/main/LruClockCache.h. The code is modified to be able to retrieve from in-memory storage for the missing value when cache miss happens. Also it is able to record information throughout the experiments.

- The Size-Aware CLOCK implementation can be found under SAClock. It is modified based on the CLOCK implementation to give it size-aware capability.

## BUILD
The project is build with CMAKE. Running `./build.sh` can build this project.

## RUN
Running `./lru_cache` will output the experiment result for storage size 1,000,000, cache size 1,000, alpha 0.1.
Policy 0 stands for LRU, 1 stands for CLOCK and 2 stands for SA-CLOCK.
