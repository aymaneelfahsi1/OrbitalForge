#include <catch2/catch_test_macros.hpp>

#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"

using orbitalforge::physics::SystemState;
using orbitalforge::simulation::integrator_name;
using orbitalforge::simulation::IntegratorKind;
using orbitalforge::simulation::parse_integrator_kind;
using orbitalforge::simulation::step_function;

TEST_CASE("integrator names parse into integrator kinds") {
  REQUIRE(parse_integrator_kind("explicit-euler") ==
          IntegratorKind::explicit_euler);

  REQUIRE(parse_integrator_kind("semi-implicit-euler") ==
          IntegratorKind::semi_implicit_euler);

  REQUIRE(parse_integrator_kind("velocity-verlet") ==
          IntegratorKind::velocity_verlet);

  REQUIRE(parse_integrator_kind("leapfrog") == IntegratorKind::leapfrog);

  REQUIRE(parse_integrator_kind("rk4") == IntegratorKind::runge_kutta_4);
}

TEST_CASE("invalid integrator names are rejected") {
  REQUIRE_FALSE(parse_integrator_kind(""));
  REQUIRE_FALSE(parse_integrator_kind("euler"));
  REQUIRE_FALSE(parse_integrator_kind("unknown"));
}

TEST_CASE("integrator kinds have stable names") {
  REQUIRE(integrator_name(IntegratorKind::explicit_euler) == "explicit-euler");

  REQUIRE(integrator_name(IntegratorKind::semi_implicit_euler) ==
          "semi-implicit-euler");

  REQUIRE(integrator_name(IntegratorKind::velocity_verlet) ==
          "velocity-verlet");

  REQUIRE(integrator_name(IntegratorKind::leapfrog) == "leapfrog");

  REQUIRE(integrator_name(IntegratorKind::runge_kutta_4) == "rk4");
}

TEST_CASE("every integrator kind maps to a step function") {
  REQUIRE(step_function(IntegratorKind::explicit_euler) != nullptr);
  REQUIRE(step_function(IntegratorKind::semi_implicit_euler) != nullptr);
  REQUIRE(step_function(IntegratorKind::velocity_verlet) != nullptr);
  REQUIRE(step_function(IntegratorKind::leapfrog) != nullptr);
  REQUIRE(step_function(IntegratorKind::runge_kutta_4) != nullptr);
}

TEST_CASE("selected integrator can advance an empty system") {
  SystemState system{};

  auto *step = step_function(IntegratorKind::velocity_verlet);

  REQUIRE_NOTHROW(step(system, 6.67430e-11, 0.0, 1.0));
}
