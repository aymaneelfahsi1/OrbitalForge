#include <catch2/catch_test_macros.hpp>

#include "orbitalforge/app/benchmark.hpp"
#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"
#include <stdexcept>
#include <vector>

using orbitalforge::app::BenchmarkBatch;
using orbitalforge::app::BenchmarkConfig;
using orbitalforge::app::BenchmarkResult;
using orbitalforge::app::BenchmarkSample;
using orbitalforge::app::parse_benchmark_body_counts;
using orbitalforge::app::run_benchmark;
using orbitalforge::app::run_benchmark_batch;
using orbitalforge::app::run_benchmark_sample;
using orbitalforge::app::Scenario;
using orbitalforge::app::validate_benchmark_config;
using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::SystemState;
using orbitalforge::simulation::IntegratorKind;

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

TEST_CASE("benchmark runs with the requested body count") {
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

  const BenchmarkResult result = run_benchmark(scenario, 4);

  REQUIRE(result.body_count == 4);
  REQUIRE(result.elapsed_milliseconds >= 0.0);
  REQUIRE(result.milliseconds_per_step >= 0.0);
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
