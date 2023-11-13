/*
 * LruClockCache.h
 *
 *  Created on: Oct 4, 2021
 *      Author: tugrul
 */

#ifndef MYCLOCKCACHE_H_
#define MYCLOCKCACHE_H_

#include <algorithm>
#include <cmath>
#include <functional>
#include <gflags/gflags.h>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "../LruClockCache/LruClockCache.h"

DECLARE_int64(init_times);

class ExponentialSmoothedThreshold {
public:
  ExponentialSmoothedThreshold(double alpha)
      : alpha_(alpha), threshold_(0.0), initialized_(false) {}

  void update_threshold(double observation) {
    if (!initialized_) {
      threshold_ = FLAGS_init_times * observation;
      initialized_ = true;
      // printf("update threshold, initialize to %lf\n", threshold_);
    } else {
      threshold_ = alpha_ * observation + (1.0 - alpha_) * threshold_;
      // printf("update threshold, update to %lf\n", threshold_);
    }
  }

  // 获取当前阈值
  inline double get_threshold() const { return threshold_; }

private:
  double alpha_;
  double threshold_;
  bool initialized_;
};

template <typename LruKey, typename LruValue,
          typename ClockHandInteger = size_t>
class MyClockCache : public LruClockCache<LruKey, LruValue, ClockHandInteger> {
public:
  MyClockCache(double alpha, ClockHandInteger numElements,
               ClockHandInteger memLimit,
               const std::function<LruValue(LruKey)> &readMiss,
               const std::function<void(LruKey, LruValue)> &writeMiss)
      : threshold_(alpha), LruClockCache<LruKey, LruValue, ClockHandInteger>(
                               numElements, memLimit, readMiss, writeMiss) {

    this->shouldAdopt = [this](size_t size) {
      // return true;
      // printf("doing shoud adopt, input sizae:%lu, threashold:%lf, waiting for
      // "
      //  "input\n",
      //  size, threshold_.get_threshold());
      // getchar();
      // bool ret = true;
      bool ret = threshold_.get_threshold() < size;
      // always update threshold
      threshold_.update_threshold(size);
      return ret;
    };
  }

private:
  ExponentialSmoothedThreshold threshold_;
};

#endif /* MYCLOCKCACHE_H_ */
