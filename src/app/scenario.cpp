#include "orbitalforge/app/scenario.hpp"

#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"

namespace orbitalforge::app {

namespace {

using physics::Body;
using simulation::IntegratorKind;

[[nodiscard]] bool is_valid_integrator(IntegratorKind integrator) noexcept {
  switch (integrator) {
  case IntegratorKind::explicit_euler:
  case IntegratorKind::semi_implicit_euler:
  case IntegratorKind::velocity_verlet:
  case IntegratorKind::leapfrog:
  case IntegratorKind::runge_kutta_4:
    return true;
  }

  return false;
}

void validate_body(const Body &body) {
  if (body.name.empty()) {
    throw std::invalid_argument{"body name must not be empty"};
  }

  if (!std::isfinite(body.mass) || body.mass <= 0.0) {
    throw std::invalid_argument{"body mass must be positive and finite"};
  }

  if (!std::isfinite(body.position.x()) || !std::isfinite(body.position.y()) ||
      !std::isfinite(body.position.z())) {
    throw std::invalid_argument{"body position must contain finite values"};
  }

  if (!std::isfinite(body.velocity.x()) || !std::isfinite(body.velocity.y()) ||
      !std::isfinite(body.velocity.z())) {
    throw std::invalid_argument{"body velocity must contain finite values"};
  }
}

} // namespace

void validate_scenario(const Scenario &scenario) {
  if (scenario.name.empty()) {
    throw std::invalid_argument{"scenario name must not be empty"};
  }

  if (!std::isfinite(scenario.gravitational_constant) ||
      scenario.gravitational_constant <= 0.0) {
    throw std::invalid_argument{
        "gravitational constant must be positive and finite"};
  }

  if (!std::isfinite(scenario.softening) || scenario.softening < 0.0) {
    throw std::invalid_argument{"softening must be non-negative and finite"};
  }

  if (!std::isfinite(scenario.time_step) || scenario.time_step <= 0.0) {
    throw std::invalid_argument{"time step must be positive and finite"};
  }

  if (scenario.step_count == 0) {
    throw std::invalid_argument{"step count must be greater than zero"};
  }

  if (scenario.output_interval == 0) {
    throw std::invalid_argument{"output interval must be greater than zero"};
  }

  if (!is_valid_integrator(scenario.integrator)) {
    throw std::invalid_argument{"scenario contains an unknown integrator"};
  }

  if (scenario.initial_state.bodies.empty()) {
    throw std::invalid_argument{"scenario must contain at least one body"};
  }

  std::unordered_set<std::string> body_names;
  body_names.reserve(scenario.initial_state.bodies.size());

  for (const Body &body : scenario.initial_state.bodies) {
    validate_body(body);

    const bool inserted = body_names.insert(body.name).second;

    if (!inserted) {
      throw std::invalid_argument{"duplicate body name: " + body.name};
    }
  }
}

} // namespace orbitalforge::app