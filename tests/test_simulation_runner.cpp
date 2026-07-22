#include <array>
#include <limits>
#include <stdexcept>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"
#include "orbitalforge/simulation/simulation_runner.hpp"

namespace {

using Catch::Approx;
using orbitalforge::math::Vec3;
using orbitalforge::physics::SystemState;
using orbitalforge::simulation::DiagnosticSample;
using orbitalforge::simulation::IntegratorKind;
using orbitalforge::simulation::SimulationConfig;
using orbitalforge::simulation::SimulationResult;
using orbitalforge::simulation::SimulationRunner;

[[nodiscard]] SystemState make_single_body_system() {
  SystemState state{};

  state.bodies.emplace_back("test-body", 1.0, Vec3{0.0, 0.0, 0.0},
                            Vec3{1.0, 0.0, 0.0});

  return state;
}

[[nodiscard]] SimulationConfig make_config() {
  return SimulationConfig{
      .gravitational_constant = 6.67430e-11,
      .softening = 0.0,
      .time_step = 0.5,
      .step_count = 4,
      .output_interval = 2,
      .integrator = IntegratorKind::explicit_euler,
  };
}

void require_vec3_approx(const Vec3 &actual, const Vec3 &expected,
                         double margin = 1e-12) {
  REQUIRE(actual.x() == Approx(expected.x()).margin(margin));
  REQUIRE(actual.y() == Approx(expected.y()).margin(margin));
  REQUIRE(actual.z() == Approx(expected.z()).margin(margin));
}

} // namespace

TEST_CASE("runner does not modify the initial state") {
  const SystemState initial_state = make_single_body_system();
  const SimulationConfig config = make_config();

  const SimulationRunner runner;
  const SimulationResult result = runner.run(initial_state, config);

  REQUIRE(initial_state.bodies.size() == 1);
  REQUIRE(initial_state.bodies[0].position == Vec3{0.0, 0.0, 0.0});
  REQUIRE(initial_state.bodies[0].velocity == Vec3{1.0, 0.0, 0.0});

  REQUIRE(result.final_system_state.bodies.size() == 1);
  REQUIRE(result.final_system_state.bodies[0].position !=
          initial_state.bodies[0].position);
}

TEST_CASE("runner records step zero") {
  const SystemState initial_state = make_single_body_system();
  const SimulationConfig config = make_config();

  const SimulationRunner runner;
  const SimulationResult result = runner.run(initial_state, config);

  REQUIRE_FALSE(result.diagnostics.empty());
  REQUIRE(result.diagnostics.front().step == 0);
  REQUIRE(result.diagnostics.front().simulation_time == 0.0);
}

TEST_CASE("runner records output intervals and final step") {
  const SystemState initial_state = make_single_body_system();

  const SimulationConfig config{
      .gravitational_constant = 6.67430e-11,
      .softening = 0.0,
      .time_step = 0.5,
      .step_count = 5,
      .output_interval = 2,
      .integrator = IntegratorKind::explicit_euler,
  };

  const SimulationRunner runner;
  const SimulationResult result = runner.run(initial_state, config);

  REQUIRE(result.diagnostics.size() == 4);

  REQUIRE(result.diagnostics[0].step == 0);
  REQUIRE(result.diagnostics[1].step == 2);
  REQUIRE(result.diagnostics[2].step == 4);
  REQUIRE(result.diagnostics[3].step == 5);
}

TEST_CASE("runner does not duplicate the final interval sample") {
  const SystemState initial_state = make_single_body_system();

  const SimulationConfig config{
      .gravitational_constant = 6.67430e-11,
      .softening = 0.0,
      .time_step = 0.5,
      .step_count = 4,
      .output_interval = 2,
      .integrator = IntegratorKind::explicit_euler,
  };

  const SimulationRunner runner;
  const SimulationResult result = runner.run(initial_state, config);

  REQUIRE(result.diagnostics.size() == 3);
  REQUIRE(result.diagnostics[0].step == 0);
  REQUIRE(result.diagnostics[1].step == 2);
  REQUIRE(result.diagnostics[2].step == 4);
}

TEST_CASE("runner reports correct simulation times") {
  const SystemState initial_state = make_single_body_system();
  const SimulationConfig config = make_config();

  const SimulationRunner runner;
  const SimulationResult result = runner.run(initial_state, config);

  REQUIRE(result.diagnostics.size() == 3);

  REQUIRE(result.diagnostics[0].simulation_time == Approx(0.0));
  REQUIRE(result.diagnostics[1].simulation_time == Approx(1.0));
  REQUIRE(result.diagnostics[2].simulation_time == Approx(2.0));
}

TEST_CASE("zero steps records only the initial state") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.step_count = 0;

  const SimulationRunner runner;
  const SimulationResult result = runner.run(initial_state, config);

  REQUIRE(result.diagnostics.size() == 1);

  const DiagnosticSample &sample = result.diagnostics.front();

  REQUIRE(sample.step == 0);
  REQUIRE(sample.simulation_time == Approx(0.0));

  REQUIRE(result.final_system_state.bodies.size() ==
          initial_state.bodies.size());

  REQUIRE(result.final_system_state.bodies[0].name ==
          initial_state.bodies[0].name);

  REQUIRE(result.final_system_state.bodies[0].mass ==
          initial_state.bodies[0].mass);

  require_vec3_approx(result.final_system_state.bodies[0].position,
                      initial_state.bodies[0].position);

  require_vec3_approx(result.final_system_state.bodies[0].velocity,
                      initial_state.bodies[0].velocity);
}

TEST_CASE("zero time step is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.time_step = 0.0;

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("negative time step is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.time_step = -0.1;

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("infinite time step is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.time_step = std::numeric_limits<double>::infinity();

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("NaN time step is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.time_step = std::numeric_limits<double>::quiet_NaN();

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("zero gravitational constant is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.gravitational_constant = 0.0;

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("negative gravitational constant is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.gravitational_constant = -1.0;

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("infinite gravitational constant is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.gravitational_constant = std::numeric_limits<double>::infinity();

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("NaN gravitational constant is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.gravitational_constant = std::numeric_limits<double>::quiet_NaN();

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("zero output interval is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.output_interval = 0;

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("empty initial system is rejected") {
  const SystemState initial_state{};
  const SimulationConfig config = make_config();

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("runner supports every integrator") {
  const SystemState initial_state = make_single_body_system();

  constexpr std::array integrators{
      IntegratorKind::explicit_euler,  IntegratorKind::semi_implicit_euler,
      IntegratorKind::velocity_verlet, IntegratorKind::leapfrog,
      IntegratorKind::runge_kutta_4,
  };

  const SimulationRunner runner;

  for (const IntegratorKind integrator : integrators) {
    SimulationConfig config = make_config();
    config.integrator = integrator;

    REQUIRE_NOTHROW(runner.run(initial_state, config));
  }
}

TEST_CASE("empty system is rejected") {
  const SystemState initial_state{};
  const SimulationConfig config = make_config();

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("negative softening is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.softening = -0.1;

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("non-finite softening is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.softening = std::numeric_limits<double>::infinity();

  const SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}
