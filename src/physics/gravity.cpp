#include "orbitalforge/physics/gravity.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace orbitalforge::physics {

using math::Vec3;

Vec3 gravitational_acceleration(const Vec3 &position,
                                double gravitational_parameter) {

  if (!std::isfinite(gravitational_parameter) ||
      gravitational_parameter <= 0.0) {
    throw std::invalid_argument{"gravitational parameter must be positive"};
  }

  const double distance_squared = position.squared_norm();

  if (distance_squared == 0.0) {
    throw std::domain_error{"gravity is undefined at the origin"};
  }

  const double distance = std::sqrt(distance_squared);

  const double scale = -gravitational_parameter / (distance_squared * distance);

  return position * scale;
}

Vec3 gravitational_acceleration(const Body &target, const Body &source,
                                double gravitational_constant) {

  if (!std::isfinite(gravitational_constant) ||
      gravitational_constant <= 0.0) {
    throw std::invalid_argument{"gravitational constant must be positive"};
  }

  const Vec3 displacement = source.position - target.position;

  const double distance_squared = displacement.squared_norm();

  if (distance_squared == 0.0) {
    throw std::domain_error{"source and target cannot be coincident"};
  }

  const double distance = std::sqrt(distance_squared);

  const double scale =
      gravitational_constant * source.mass / (distance_squared * distance);

  return displacement * scale;
}

std::vector<Vec3> gravitational_accelerations(const SystemState &system,
                                              double gravitational_constant) {

  if (!std::isfinite(gravitational_constant) ||
      gravitational_constant <= 0.0) {
    throw std::invalid_argument{"gravitational constant must be positive"};
  }

  std::vector<Vec3> accelerations(system.bodies.size(), Vec3{});

  for (std::size_t first_index = 0; first_index < system.bodies.size();
       ++first_index) {

    for (std::size_t second_index = first_index + 1;
         second_index < system.bodies.size(); ++second_index) {

      const Body &first_body = system.bodies[first_index];

      const Body &second_body = system.bodies[second_index];

      const Vec3 displacement = second_body.position - first_body.position;

      const double distance_squared = displacement.squared_norm();

      if (distance_squared == 0.0) {
        throw std::domain_error{"two bodies cannot occupy the same position"};
      }

      const double distance = std::sqrt(distance_squared);

      const double common_scale =
          gravitational_constant / (distance_squared * distance);

      accelerations[first_index] +=
          displacement * (common_scale * second_body.mass);

      accelerations[second_index] -=
          displacement * (common_scale * first_body.mass);
    }
  }

  return accelerations;
}

} // namespace orbitalforge::physics
