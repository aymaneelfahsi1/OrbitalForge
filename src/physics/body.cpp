#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/math/vec3.hpp"

#include <cmath>
#include <stdexcept>
#include <utility>

using orbitalforge::math::Vec3;

namespace orbitalforge::physics {

Body::Body(std::string body_name, double body_mass, Vec3 body_position,
           Vec3 body_velocity)
    : name{std::move(body_name)}, mass{body_mass}, position{body_position},
      velocity{body_velocity} {
  if (!std::isfinite(mass) || mass <= 0.0) {
    throw std::invalid_argument{"body mass must be positive"};
  }
}

State Body::state() const noexcept { return State{position, velocity}; };

void Body::set_state(const State &new_state) noexcept {
  position = new_state.position;
  velocity = new_state.velocity;
};
} // namespace orbitalforge::physics
