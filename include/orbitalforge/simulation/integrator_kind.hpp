#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "orbitalforge/physics/system_state.hpp"

namespace orbitalforge::simulation {

enum class IntegratorKind {
  explicit_euler,
  semi_implicit_euler,
  velocity_verlet,
  leapfrog,
  runge_kutta_4

};

using StepFunction = void(physics::SystemState &, double, double);

[[nodiscard]] std::optional<IntegratorKind>
parse_integrator_kind(std::string_view value);

[[nodiscard]] std::string_view integrator_name(IntegratorKind kind);
[[nodiscard]] StepFunction *step_function(IntegratorKind kind);

} // namespace orbitalforge::simulation