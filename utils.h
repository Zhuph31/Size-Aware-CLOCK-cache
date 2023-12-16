#pragma once
#include <chrono>
#include <random>
#include <stdarg.h>
#include <string>

class TimeCost {
public:
  TimeCost() : start_time(std::chrono::high_resolution_clock::now()) {}

  size_t get_elapsed() const {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    return duration.count();
  }

private:
  std::chrono::high_resolution_clock::time_point start_time;
};

inline std::string generateRandomString() {
  const std::string charset =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  std::default_random_engine randomEngine(std::random_device{}());

  std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

  std::string randomString;
  int length = distribution(randomEngine);

  for (int i = 0; i < length; ++i) {
    randomString += charset[distribution(randomEngine)];
  }

  return randomString;
}

std::string string_printf(const char *format, ...);