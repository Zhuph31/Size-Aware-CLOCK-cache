#pragma once
#include <random>
#include <string>

std::string generateRandomString() {
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