#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/math/vec3.hpp"

#include <stdexcept>
#include <utility>

using orbitalforge::math::Vec3;

namespace orbitalforge::physics {

Body::Body(std::string name, double mass, Vec3 position, Vec3 velocity)
    : name{std::move(name)}, mass{mass}, position{position},
      velocity{velocity} {
  if (this->mass <= 0.0) {
    throw std::invalid_argument{"body mass must be positive"};
  }
}

State Body::state() const noexcept { return State{position, velocity}; };

void Body::set_state(const State &new_state) noexcept {
  position = new_state.position;
  velocity = new_state.velocity;
};
} // namespace orbitalforge::physics