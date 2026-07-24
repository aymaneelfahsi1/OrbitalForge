#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <stdexcept>
#include <vector>

#include "orbitalforge/statistics/sample_statistics.hpp"

using orbitalforge::statistics::SampleStatistics;
using orbitalforge::statistics::summarize_samples;

TEST_CASE("statistics reject empty samples") {
  REQUIRE_THROWS_AS(summarize_samples({}), std::invalid_argument);
}

TEST_CASE("statistics calculate minimum and maximum") {
  const SampleStatistics result = summarize_samples({4.0, 1.0, 9.0, 2.0});

  REQUIRE(result.minimum == 1.0);
  REQUIRE(result.maximum == 9.0);
}

TEST_CASE("statistics calculate odd median") {
  const SampleStatistics result = summarize_samples({9.0, 1.0, 5.0});

  REQUIRE(result.median == 5.0);
}

TEST_CASE("statistics calculate even median") {
  const SampleStatistics result = summarize_samples({8.0, 2.0, 6.0, 4.0});

  REQUIRE(result.median == 5.0);
}

TEST_CASE("statistics calculate mean") {
  const SampleStatistics result = summarize_samples({2.0, 4.0, 6.0, 8.0});

  REQUIRE(result.mean == 5.0);
}

TEST_CASE("one sample has zero standard deviation") {
  const SampleStatistics result = summarize_samples({7.5});

  REQUIRE(result.standard_deviation == 0.0);
}

TEST_CASE("statistics calculate sample standard deviation") {
  const SampleStatistics result = summarize_samples({2.0, 4.0, 6.0});

  REQUIRE(result.standard_deviation == std::sqrt(4.0));
}

TEST_CASE("statistics calculate p95") {
  const SampleStatistics result =
      summarize_samples({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0});

  REQUIRE(result.p95 == 10.0);
}

TEST_CASE("statistics preserve the input samples") {
  const std::vector<double> samples{4.0, 1.0, 3.0};
  const std::vector<double> original = samples;

  static_cast<void>(summarize_samples(samples));

  REQUIRE(samples == original);
}
