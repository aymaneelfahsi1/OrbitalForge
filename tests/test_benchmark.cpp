#include <catch2/catch_test_macros.hpp>

#include "orbitalforge/app/benchmark.hpp"
#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"
#include "orbitalforge/statistics/sample_statistics.hpp"
#include <stdexcept>
#include <vector>

using orbitalforge::app::benchmark_all_integrators;
using orbitalforge::app::BenchmarkBatch;
using orbitalforge::app::BenchmarkConfig;
using orbitalforge::app::BenchmarkSample;
using orbitalforge::app::IntegratorBenchmarkResult;
using orbitalforge::app::parse_benchmark_body_counts;
using orbitalforge::app::run_benchmark_batch;
using orbitalforge::app::run_benchmark_sample;
using orbitalforge::app::Scenario;
using orbitalforge::app::sort_integrator_results;
using orbitalforge::app::stable_sort_integrator_results;
using orbitalforge::app::summarize_benchmark_batch;
using orbitalforge::app::validate_benchmark_config;
using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::SystemState;
using orbitalforge::simulation::IntegratorKind;
using orbitalforge::statistics::SampleStatistics;

TEST_CASE("benchmark body counts are parsed") {
  REQUIRE(parse_benchmark_body_counts("1,8,64") ==
          std::vector<std::size_t>{1, 8, 64});
}

TEST_CASE("benchmark body counts reject invalid values") {
  REQUIRE_THROWS_AS(parse_benchmark_body_counts(""), std::invalid_argument);
  REQUIRE_THROWS_AS(parse_benchmark_body_counts("1,0,4"),
                    std::invalid_argument);
  REQUIRE_THROWS_AS(parse_benchmark_body_counts("1,size,4"),
                    std::invalid_argument);
}

TEST_CASE("benchmark configuration rejects zero warmups") {
  const BenchmarkConfig config{
      .warmup_count = 0,
      .repetition_count = 3,
      .body_count = 4,
      .step_count = 2,
      .integrator_kind = IntegratorKind::velocity_verlet,
  };

  REQUIRE_THROWS_AS(validate_benchmark_config(config), std::invalid_argument);
}

TEST_CASE("benchmark configuration rejects zero repetitions") {
  const BenchmarkConfig config{
      .warmup_count = 2,
      .repetition_count = 0,
      .body_count = 4,
      .step_count = 2,
      .integrator_kind = IntegratorKind::velocity_verlet,
  };

  REQUIRE_THROWS_AS(validate_benchmark_config(config), std::invalid_argument);
}

TEST_CASE("benchmark configuration rejects zero bodies") {
  const BenchmarkConfig config{
      .warmup_count = 2,
      .repetition_count = 3,
      .body_count = 0,
      .step_count = 2,
      .integrator_kind = IntegratorKind::velocity_verlet,
  };

  REQUIRE_THROWS_AS(validate_benchmark_config(config), std::invalid_argument);
}

TEST_CASE("benchmark configuration rejects zero steps") {
  const BenchmarkConfig config{
      .warmup_count = 2,
      .repetition_count = 3,
      .body_count = 4,
      .step_count = 0,
      .integrator_kind = IntegratorKind::velocity_verlet,
  };

  REQUIRE_THROWS_AS(validate_benchmark_config(config), std::invalid_argument);
}

TEST_CASE("benchmark configuration accepts valid values") {
  const BenchmarkConfig config{
      .warmup_count = 2,
      .repetition_count = 3,
      .body_count = 4,
      .step_count = 2,
      .integrator_kind = IntegratorKind::velocity_verlet,
  };

  REQUIRE_NOTHROW(validate_benchmark_config(config));
}

TEST_CASE("benchmark sample preserves configuration metadata") {
  const Scenario scenario{
      .name = "Benchmark",
      .gravitational_constant = 1.0,
      .softening = 0.0,
      .time_step = 0.01,
      .step_count = 2,
      .output_interval = 1,
      .integrator = IntegratorKind::velocity_verlet,
      .seed = 1,
      .initial_state =
          SystemState{
              .bodies{
                  Body{"Template", 1.0, Vec3{}, Vec3{1.0, 0.0, 0.0}},
              },
          },
  };

  const BenchmarkConfig config{
      .warmup_count = 1,
      .repetition_count = 2,
      .body_count = 4,
      .step_count = 2,
      .integrator_kind = IntegratorKind::velocity_verlet,
  };

  const BenchmarkSample sample = run_benchmark_sample(scenario, config);

  REQUIRE(sample.body_count == 4);
  REQUIRE(sample.step_count == 2);
  REQUIRE(sample.integrator_kind == IntegratorKind::velocity_verlet);
  REQUIRE(sample.elapsed_milliseconds >= 0.0);
  REQUIRE(sample.milliseconds_per_step >= 0.0);
}

TEST_CASE("benchmark batch excludes warmup samples") {
  const Scenario scenario{
      .name = "Benchmark",
      .gravitational_constant = 1.0,
      .softening = 0.0,
      .time_step = 0.01,
      .step_count = 2,
      .output_interval = 1,
      .integrator = IntegratorKind::velocity_verlet,
      .seed = 1,
      .initial_state =
          SystemState{
              .bodies{
                  Body{"Template", 1.0, Vec3{}, Vec3{1.0, 0.0, 0.0}},
              },
          },
  };

  const BenchmarkConfig config{
      .warmup_count = 2,
      .repetition_count = 5,
      .body_count = 4,
      .step_count = 2,
      .integrator_kind = IntegratorKind::velocity_verlet,
  };

  const BenchmarkBatch batch = run_benchmark_batch(scenario, config);

  REQUIRE(batch.samples.size() == 5);
  REQUIRE(batch.checksum != 0);
}

TEST_CASE("benchmark batch retains exactly requested repetitions") {
  const Scenario scenario{
      .name = "Benchmark",
      .gravitational_constant = 1.0,
      .softening = 0.0,
      .time_step = 0.01,
      .step_count = 1,
      .output_interval = 1,
      .integrator = IntegratorKind::velocity_verlet,
      .seed = 1,
      .initial_state =
          SystemState{
              .bodies{
                  Body{"Template", 1.0, Vec3{}, Vec3{}},
              },
          },
  };

  const BenchmarkConfig config{
      .warmup_count = 1,
      .repetition_count = 3,
      .body_count = 2,
      .step_count = 1,
      .integrator_kind = IntegratorKind::velocity_verlet,
  };

  const BenchmarkBatch batch = run_benchmark_batch(scenario, config);

  REQUIRE(batch.samples.size() == 3);

  for (const BenchmarkSample &sample : batch.samples) {
    REQUIRE(sample.body_count == 2);
    REQUIRE(sample.step_count == 1);
    REQUIRE(sample.elapsed_milliseconds >= 0.0);
    REQUIRE(sample.milliseconds_per_step >= 0.0);
  }
}

TEST_CASE("benchmark batch statistics summarize measured samples") {
  const BenchmarkBatch batch{
      .samples{
          BenchmarkSample{
              .body_count = 4,
              .step_count = 2,
              .integrator_kind = IntegratorKind::velocity_verlet,
              .elapsed_milliseconds = 4.0,
              .milliseconds_per_step = 2.0,
              .checksum = 10,
          },
          BenchmarkSample{
              .body_count = 4,
              .step_count = 2,
              .integrator_kind = IntegratorKind::velocity_verlet,
              .elapsed_milliseconds = 2.0,
              .milliseconds_per_step = 1.0,
              .checksum = 20,
          },
          BenchmarkSample{
              .body_count = 4,
              .step_count = 2,
              .integrator_kind = IntegratorKind::velocity_verlet,
              .elapsed_milliseconds = 6.0,
              .milliseconds_per_step = 3.0,
              .checksum = 30,
          },
      },
      .checksum = 0,
  };

  const auto statistics = summarize_benchmark_batch(batch);

  REQUIRE(statistics.minimum == 2.0);
  REQUIRE(statistics.maximum == 6.0);
  REQUIRE(statistics.median == 4.0);
  REQUIRE(statistics.mean == 4.0);
}

TEST_CASE("benchmark batch statistics reject empty batches") {
  const BenchmarkBatch batch{
      .samples{},
      .checksum = 0,
  };

  REQUIRE_THROWS_AS(summarize_benchmark_batch(batch), std::invalid_argument);
}

TEST_CASE("benchmark batch produces summarized runtime statistics") {
  const Scenario scenario{
      .name = "Benchmark",
      .gravitational_constant = 1.0,
      .softening = 0.0,
      .time_step = 0.01,
      .step_count = 2,
      .output_interval = 1,
      .integrator = IntegratorKind::velocity_verlet,
      .seed = 1,
      .initial_state =
          SystemState{
              .bodies{
                  Body{"Template", 1.0, Vec3{}, Vec3{}},
              },
          },
  };

  const BenchmarkConfig config{
      .warmup_count = 1,
      .repetition_count = 4,
      .body_count = 4,
      .step_count = 2,
      .integrator_kind = IntegratorKind::velocity_verlet,
  };

  const BenchmarkBatch batch = run_benchmark_batch(scenario, config);

  const auto statistics = summarize_benchmark_batch(batch);

  REQUIRE(batch.samples.size() == 4);
  REQUIRE(statistics.minimum >= 0.0);
  REQUIRE(statistics.maximum >= statistics.minimum);
  REQUIRE(statistics.median >= statistics.minimum);
  REQUIRE(statistics.median <= statistics.maximum);
  REQUIRE(statistics.mean >= statistics.minimum);
  REQUIRE(statistics.mean <= statistics.maximum);
  REQUIRE(statistics.standard_deviation >= 0.0);
  REQUIRE(statistics.p95 >= statistics.minimum);
  REQUIRE(statistics.p95 <= statistics.maximum);
}

TEST_CASE("benchmark all integrators produces one result per integrator") {
  const Scenario scenario{
      .name = "Benchmark",
      .gravitational_constant = 1.0,
      .softening = 0.0,
      .time_step = 0.01,
      .step_count = 2,
      .output_interval = 1,
      .integrator = IntegratorKind::velocity_verlet,
      .seed = 1,
      .initial_state =
          SystemState{
              .bodies{
                  Body{"Template", 1.0, Vec3{}, Vec3{}},
              },
          },
  };

  const std::vector<IntegratorBenchmarkResult> results =
      benchmark_all_integrators(scenario, 4, 1, 2);

  REQUIRE(results.size() == 5);

  for (const IntegratorBenchmarkResult &result : results) {
    REQUIRE(result.body_count == 4);
    REQUIRE(result.step_count == 2);
    REQUIRE(result.timing.minimum >= 0.0);
    REQUIRE(result.timing.maximum >= result.timing.minimum);
    REQUIRE(result.timing.mean >= result.timing.minimum);
    REQUIRE(result.timing.p95 >= result.timing.minimum);
  }
}

TEST_CASE("integrator results sort by mean runtime") {
  const SampleStatistics slow{
      .minimum = 8.0,
      .maximum = 12.0,
      .median = 10.0,
      .mean = 10.0,
      .standard_deviation = 1.0,
      .p95 = 12.0,
  };

  const SampleStatistics fast{
      .minimum = 2.0,
      .maximum = 4.0,
      .median = 3.0,
      .mean = 3.0,
      .standard_deviation = 0.5,
      .p95 = 4.0,
  };

  std::vector<IntegratorBenchmarkResult> results{
      IntegratorBenchmarkResult{
          .integrator_kind = IntegratorKind::runge_kutta_4,
          .body_count = 4,
          .step_count = 10,
          .timing = slow,
          .checksum = 1,
      },
      IntegratorBenchmarkResult{
          .integrator_kind = IntegratorKind::explicit_euler,
          .body_count = 4,
          .step_count = 10,
          .timing = fast,
          .checksum = 2,
      },
  };

  sort_integrator_results(results);

  REQUIRE(results.front().integrator_kind == IntegratorKind::explicit_euler);

  REQUIRE(results.back().integrator_kind == IntegratorKind::runge_kutta_4);
}

TEST_CASE("integrator result sorting uses deterministic tie breaking") {
  const SampleStatistics equal_statistics{
      .minimum = 3.0,
      .maximum = 3.0,
      .median = 3.0,
      .mean = 3.0,
      .standard_deviation = 0.0,
      .p95 = 3.0,
  };

  std::vector<IntegratorBenchmarkResult> results{
      IntegratorBenchmarkResult{
          .integrator_kind = IntegratorKind::runge_kutta_4,
          .body_count = 4,
          .step_count = 10,
          .timing = equal_statistics,
          .checksum = 1,
      },
      IntegratorBenchmarkResult{
          .integrator_kind = IntegratorKind::explicit_euler,
          .body_count = 4,
          .step_count = 10,
          .timing = equal_statistics,
          .checksum = 2,
      },
  };

  sort_integrator_results(results);

  REQUIRE(results.front().integrator_kind == IntegratorKind::explicit_euler);

  REQUIRE(results.back().integrator_kind == IntegratorKind::runge_kutta_4);
}

TEST_CASE("stable integrator result sorting preserves equal runtime order") {
  const SampleStatistics equal_statistics{
      .minimum = 3.0,
      .maximum = 3.0,
      .median = 3.0,
      .mean = 3.0,
      .standard_deviation = 0.0,
      .p95 = 3.0,
  };

  std::vector<IntegratorBenchmarkResult> results{
      IntegratorBenchmarkResult{
          .integrator_kind = IntegratorKind::runge_kutta_4,
          .body_count = 4,
          .step_count = 10,
          .timing = equal_statistics,
          .checksum = 1,
      },
      IntegratorBenchmarkResult{
          .integrator_kind = IntegratorKind::explicit_euler,
          .body_count = 4,
          .step_count = 10,
          .timing = equal_statistics,
          .checksum = 2,
      },
  };

  stable_sort_integrator_results(results);

  REQUIRE(results.front().integrator_kind == IntegratorKind::runge_kutta_4);
  REQUIRE(results.back().integrator_kind == IntegratorKind::explicit_euler);
}
