#include "orbitalforge/physics/integrator.hpp"
#include "orbitalforge/math/vec3.hpp"

#include <cmath>
#include <stdexcept>

namespace orbitalforge::physics {

State explicit_euler_step(const State &state, const math::Vec3 &acceleration,
                          double time_step) {

  if (!std::isfinite(time_step) || time_step <= 0.0) {
    throw std::invalid_argument("time step must be positive");
  }

  const math::Vec3 next_position = state.position + state.velocity * time_step;

  const math::Vec3 next_velocity = state.velocity + acceleration * time_step;

  return State{next_position, next_velocity};
}

State semi_implicit_euler_step(const State &state,
                               const math::Vec3 &acceleration,
                               double time_step) {

  if (!std::isfinite(time_step) || time_step <= 0.0) {
    throw std::invalid_argument("time step must be positive");
  }

  const math::Vec3 next_velocity = state.velocity + acceleration * time_step;
  const math::Vec3 next_position = state.position + next_velocity * time_step;

  return State{next_position, next_velocity};
}

} // namespace orbitalforge::physics
