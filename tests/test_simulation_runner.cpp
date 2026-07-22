#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"
#include "orbitalforge/simulation/simulation_runner.hpp"

using orbitalforge::math::Vec3;
using orbitalforge::physics::SystemState;
using orbitalforge::simulation::IntegratorKind;
using orbitalforge::simulation::SimulationConfig;
using orbitalforge::simulation::SimulationRunner;

namespace {

SystemState make_single_body_system() {
  SystemState state{};

  state.bodies.emplace_back("test-body", 1.0, Vec3{0.0, 0.0, 0.0},
                            Vec3{1.0, 0.0, 0.0});

  return state;
}

SimulationConfig make_config() {
  return SimulationConfig{
      .gravitational_constant = 6.67430e-11,
      .time_step = 0.5,
      .step_count = 4,
      .output_interval = 2,
      .integrator = IntegratorKind::explicit_euler,
  };
}

} // namespace

TEST_CASE("runner does not modify the initial state") {
  const SystemState initial_state = make_single_body_system();
  const SimulationConfig config = make_config();

  SimulationRunner runner;

  const auto result = runner.run(initial_state, config);

  REQUIRE(initial_state.bodies.size() == 1);
  REQUIRE(initial_state.bodies[0].position == Vec3{0.0, 0.0, 0.0});
  REQUIRE(initial_state.bodies[0].velocity == Vec3{1.0, 0.0, 0.0});

  REQUIRE(result.final_system_state.bodies[0].position !=
          initial_state.bodies[0].position);
}

TEST_CASE("runner records step zero") {
  const SystemState initial_state = make_single_body_system();
  const SimulationConfig config = make_config();

  SimulationRunner runner;

  const auto result = runner.run(initial_state, config);

  REQUIRE_FALSE(result.diagnostics.empty());
  REQUIRE(result.diagnostics.front().step == 0);
  REQUIRE(result.diagnostics.front().simulation_time == 0.0);
}

TEST_CASE("runner records output intervals and final step") {
  const SystemState initial_state = make_single_body_system();

  const SimulationConfig config{
      .gravitational_constant = 6.67430e-11,
      .time_step = 0.5,
      .step_count = 5,
      .output_interval = 2,
      .integrator = IntegratorKind::explicit_euler,
  };

  SimulationRunner runner;

  const auto result = runner.run(initial_state, config);

  REQUIRE(result.diagnostics.size() == 4);

  REQUIRE(result.diagnostics[0].step == 0);
  REQUIRE(result.diagnostics[1].step == 2);
  REQUIRE(result.diagnostics[2].step == 4);
  REQUIRE(result.diagnostics[3].step == 5);
}

TEST_CASE("runner reports correct simulation times") {
  const SystemState initial_state = make_single_body_system();
  const SimulationConfig config = make_config();

  SimulationRunner runner;

  const auto result = runner.run(initial_state, config);

  REQUIRE(result.diagnostics.size() == 3);

  REQUIRE(result.diagnostics[0].simulation_time == 0.0);
  REQUIRE(result.diagnostics[1].simulation_time == 1.0);
  REQUIRE(result.diagnostics[2].simulation_time == 2.0);
}

TEST_CASE("zero steps returns unchanged copied state") {
  const SystemState initial_state = make_single_body_system();

  const SimulationConfig config{
      .gravitational_constant = 6.67430e-11,
      .time_step = 0.5,
      .step_count = 0,
      .output_interval = 1,
      .integrator = IntegratorKind::velocity_verlet,
  };

  SimulationRunner runner;

  const auto result = runner.run(initial_state, config);

  REQUIRE(result.final_system_state.bodies.size() == 1);
  REQUIRE(result.final_system_state.bodies[0].position ==
          initial_state.bodies[0].position);

  REQUIRE(result.final_system_state.bodies[0].velocity ==
          initial_state.bodies[0].velocity);

  REQUIRE(result.diagnostics.size() == 1);
  REQUIRE(result.diagnostics[0].step == 0);
}

TEST_CASE("invalid time step is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.time_step = 0.0;

  SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("invalid gravitational constant is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.gravitational_constant = -1.0;

  SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("zero output interval is rejected") {
  const SystemState initial_state = make_single_body_system();
  SimulationConfig config = make_config();

  config.output_interval = 0;

  SimulationRunner runner;

  REQUIRE_THROWS_AS(runner.run(initial_state, config), std::invalid_argument);
}

TEST_CASE("runner supports every integrator") {
  const SystemState initial_state = make_single_body_system();

  const IntegratorKind integrators[] = {
      IntegratorKind::explicit_euler,  IntegratorKind::semi_implicit_euler,
      IntegratorKind::velocity_verlet, IntegratorKind::leapfrog,
      IntegratorKind::runge_kutta_4,
  };

  SimulationRunner runner;

  for (const IntegratorKind integrator : integrators) {
    SimulationConfig config = make_config();
    config.integrator = integrator;

    REQUIRE_NOTHROW(runner.run(initial_state, config));
  }
}