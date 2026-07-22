#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <vector>

#include "orbitalforge/app/benchmark.hpp"
#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"

using orbitalforge::app::BenchmarkResult;
using orbitalforge::app::parse_benchmark_body_counts;
using orbitalforge::app::run_benchmark;
using orbitalforge::app::Scenario;
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
