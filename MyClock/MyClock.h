/*
 * LruClockCache.h
 *
 *  Created on: Oct 4, 2021
 *      Author: tugrul
 */

#ifndef MYCLOCKCACHE_H_
#define MYCLOCKCACHE_H_

#include "../LruClockCache/LruClockCache.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

class ExponentialSmoothedThreshold {
public:
  ExponentialSmoothedThreshold(double alpha)
      : alpha_(alpha), threshold_(0.0), initialized_(false) {}

  void update_threshold(double observation) {
    if (!initialized_) {
      threshold_ = observation;
      initialized_ = true;
    } else {
      threshold_ = alpha_ * observation + (1.0 - alpha_) * threshold_;
    }
  }

  // 获取当前阈值
  double get_threshold() const { return threshold_; }

private:
  double alpha_;
  double threshold_;
  bool initialized_;
};

template <typename LruKey, typename LruValue,
          typename ClockHandInteger = size_t>
class MyClockCache : public LruClockCache<LruKey, LruValue> {
public:
  MyClockCache(double alpha, ClockHandInteger numElements,
               const std::function<LruValue(LruKey)> &readMiss,
               const std::function<void(LruKey, LruValue)> &writeMiss)
      : LruClockCache<LruKey, LruValue>(numElements, readMiss, writeMiss),
        threshold_(alpha) {
    this->check_size = [this](size_t s) {
      bool pass = s < threshold_.get_threshold();
      if (!pass) {
        printf("reject, %lu vs %lu\n", s, threshold_.get_threshold());
      }
      return pass;
    };
  }

  inline const LruValue get(const LruKey &key) noexcept override {
    LruValue value = this->accessClock2Hand(key, nullptr);
    // always update threshold
    threshold_.update_threshold(key.size());
    return value;
  }

private:
  ExponentialSmoothedThreshold threshold_;
};

#endif /* MYCLOCKCACHE_H_ */
