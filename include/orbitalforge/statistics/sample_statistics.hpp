#pragma once

#include <cstddef>
#include <vector>

namespace orbitalforge::statistics {

struct SampleStatistics {
  double minimum;
  double maximum;
  double median;
  double mean;
  double standard_deviation;
  double p95;
};

[[nodiscard]] SampleStatistics
summarize_samples(const std::vector<double> &samples);

} // namespace orbitalforge::statistics