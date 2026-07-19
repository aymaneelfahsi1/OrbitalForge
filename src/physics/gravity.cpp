#include "orbitalforge/physics/gravity.hpp"
#include <stdexcept>

namespace orbitalforge::physics {

math::Vec3 gravitational_acceleration(const math::Vec3 &position,
                                      double gravitational_parameter) {
  if (gravitational_parameter <= .0) {
    throw std::invalid_argument("gravitational_parameter cannot be negative!");
  }

  const double distance = position.norm();

  if (distance == 0) {
    throw std::domain_error("gravity is undefined at the origin");
  }

  const double scale =
      -gravitational_parameter / (distance * distance * distance);

  return position * scale;
}

} // namespace orbitalforge::physics