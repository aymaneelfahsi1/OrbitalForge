#include "orbitalforge/simulation/integrator_kind.hpp"

#include "stdexcept"

#include "orbitalforge/simulation/step.hpp"
#include <optional>
#include <stdexcept>
#include <string_view>

namespace orbitalforge::simulation {

using orbitalforge::simulation::IntegratorKind;

std::optional<IntegratorKind> parse_integrator_kind(std::string_view value) {
  if (value == "explicit-euler") {
    return IntegratorKind::explicit_euler;
  }

  if (value == "semi-implicit-euler") {
    return IntegratorKind::semi_implicit_euler;
  }

  if (value == "velocity-verlet") {
    return IntegratorKind::velocity_verlet;
  }

  if (value == "leapfrog") {
    return IntegratorKind::leapfrog;
  }

  if (value == "rk4") {
    return IntegratorKind::runge_kutta_4;
  }

  return std::nullopt;
}

std::string_view integrator_name(IntegratorKind kind) {
  switch (kind) {

  case IntegratorKind::explicit_euler:
    return "explicit-euler";
  case IntegratorKind::semi_implicit_euler:
    return "semi-implicit-euler";
  case IntegratorKind::velocity_verlet:
    return "velocity-verlet";
  case IntegratorKind::leapfrog:
    return "leapfrog";
  case IntegratorKind::runge_kutta_4:
    return "rk4";
  }

  throw std::invalid_argument{"invalid integrator kind"};
}

StepFunction *step_function(IntegratorKind kind) {
  switch (kind) {
  case IntegratorKind::explicit_euler:
    return advance_explicit_euler_step;
  case IntegratorKind::semi_implicit_euler:
    return advance_semi_implicit_euler_step;
  case IntegratorKind::velocity_verlet:
    return advance_velocity_verlet_step;
  case IntegratorKind::leapfrog:
    return advance_leapfrog_step;
  case IntegratorKind::runge_kutta_4:
    return advance_runge_kutta_4_step;
  }

  throw std::invalid_argument{"invalid integrator kind"};
}

} // namespace orbitalforge::simulation