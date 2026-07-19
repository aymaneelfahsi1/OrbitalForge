#include "orbitalforge/physics/gravity.hpp"
#include <cstddef>
#include <stdexcept>
#include <vector>

namespace orbitalforge::physics {

using math::Vec3;

using std::vector;

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

vector<Vec3> gravitational_accelerations(const SystemState &system,
                                         double gravitational_constant) {

  vector<Vec3> accelerations(system.bodies.size(), Vec3{});

  for (std::size_t target_index = 0; target_index < system.bodies.size();
       ++target_index) {

    for (std::size_t source_index = 0; source_index < system.bodies.size();
         ++source_index) {
      if (target_index == source_index) {
        continue;
      }

      accelerations[target_index] += gravitational_acceleration(
          system.bodies[target_index], system.bodies[source_index],
          gravitational_constant);
    }
  }

  return accelerations;
}

} // namespace orbitalforge::physics