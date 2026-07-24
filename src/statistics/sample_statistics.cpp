#include "orbitalforge/statistics/sample_statistics.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace orbitalforge::statistics {

namespace {

[[nodiscard]] double calculate_mean(const std::vector<double> &samples) {
  const double total = std::accumulate(samples.begin(), samples.end(), 0);

  return total / static_cast<double>(samples.size());
}

[[nodiscard]] double
calculate_median(const std::vector<double> &sorted_samples) {

  const std::size_t middle = sorted_samples.size() / 2;

  if (sorted_samples.size() % 2 == 1) {
    return sorted_samples[middle];
  }

  return (sorted_samples[middle - 1] + sorted_samples[middle]) / 2.0;
}

[[nodiscard]] double
calculate_standard_deviation(const std::vector<double> &samples, double mean) {
  if (samples.size() == 1) {
    return 0.0;
  }

  double squared_difference_total = 0.0;

  for (const double sample : samples) {
    const double difference = sample - mean;
    squared_difference_total += difference * difference;
  }

  return std::sqrt(squared_difference_total /
                   static_cast<double>(samples.size() - 1));
}

[[nodiscard]] double calculate_p95(const std::vector<double> &sorted_samples) {
  const double last_index = static_cast<double>(sorted_samples.size() - 1);

  const double raw_index = 0.95 * last_index;

  const std::size_t p95_index = static_cast<std::size_t>(std::ceil(raw_index));

  return sorted_samples[p95_index];
}
} // namespace

SampleStatistics summarize_samples(const std::vector<double> &samples) {
  if (samples.empty()) {
    throw std::invalid_argument{"cannort summarize empty samples"};
  }

  std::vector<double> sorted_samples = samples;
  std::sort(sorted_samples.begin(), sorted_samples.end());

  const double mean = calculate_mean(samples);

  return SampleStatistics{
      .minimum = sorted_samples.front(),
      .maximum = sorted_samples.back(),
      .median = calculate_median(sorted_samples),
      .mean = mean,
      .standard_deviation = calculate_standard_deviation(samples, mean),
      .p95 = calculate_p95(sorted_samples),
  };
}
} // namespace orbitalforge::statistics