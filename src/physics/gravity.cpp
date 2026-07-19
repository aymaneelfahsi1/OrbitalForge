#include "orbitalforge/physics/gravity.hpp"
#include <stdexcept>

namespace orbitalforge::physics {

using math::Vec3;

Vec3 gravitational_acceleration(const math::Vec3 &position,
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

Vec3 gravitational_acceleration(const Body &target, const Body &source,
                                double gravitational_constant) {
  if (gravitational_constant <= .0) {
    throw std::invalid_argument("gravitational_parameter cannot be negative!");
  }

  const Vec3 displacement = source.position - target.position;
  const double distance_squared = displacement.squared_norm();
  const double distance = displacement.norm();

  if (distance == .0) {
    throw std::domain_error{"Source and target can't be coincident"};
  }

  return displacement *
         (gravitational_constant * source.mass / (distance_squared * distance));
}

} // namespace orbitalforge::physics