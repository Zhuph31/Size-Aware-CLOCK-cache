#pragma once
#include <chrono>
#include <random>
#include <string>

class TimeCost {
public:
  // 构造函数，记录当前时间
  TimeCost() : start_time(std::chrono::high_resolution_clock::now()) {}

  // 获取自构造以来经过的毫秒数
  long long get_elapsed() const {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    return duration.count();
  }

private:
  std::chrono::high_resolution_clock::time_point start_time;
};

inline std::string generateRandomString() {
  // 定义字符集
  const std::string charset =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  // 使用 C++11 的随机数引擎
  std::default_random_engine randomEngine(std::random_device{}());

  // 使用字符集定义字符分布
  std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

  // 生成随机字符串
  std::string randomString;
  int length = distribution(randomEngine);

  for (int i = 0; i < length; ++i) {
    randomString += charset[distribution(randomEngine)];
  }

  return randomString;
}