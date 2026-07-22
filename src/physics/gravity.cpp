#include "orbitalforge/physics/gravity.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace orbitalforge::physics {

namespace {

void validate_parameters(double gravitational_value, double softening) {
  if (!std::isfinite(gravitational_value) || gravitational_value <= 0.0) {
    throw std::invalid_argument{
        "gravitational parameter must be positive and finite"};
  }

  if (!std::isfinite(softening) || softening < 0.0) {
    throw std::invalid_argument{"softening must be non-negative and finite"};
  }
}

} // namespace

Vec3 gravitational_acceleration(const Vec3 &position,
                                double gravitational_parameter,
                                double softening) {
  validate_parameters(gravitational_parameter, softening);

  const double softened_squared_distance =
      position.squared_norm() + softening * softening;

  if (softened_squared_distance == 0.0) {
    throw std::domain_error{
        "gravitational acceleration is undefined at the origin"};
  }

  const double softened_distance = std::sqrt(softened_squared_distance);

  const double inverse_distance_cubed =
      1.0 / (softened_squared_distance * softened_distance);

  return position * (-gravitational_parameter * inverse_distance_cubed);
}

Vec3 gravitational_acceleration(const Body &target, const Body &source,
                                double gravitational_constant,
                                double softening) {
  const Vec3 displacement = target.position - source.position;

  return gravitational_acceleration(
      displacement, gravitational_constant * source.mass, softening);
}

std::vector<Vec3> gravitational_accelerations(const SystemState &system,
                                              double gravitational_constant,
                                              double softening) {
  validate_parameters(gravitational_constant, softening);

  std::vector<Vec3> accelerations(system.bodies.size());

  for (std::size_t first = 0; first < system.bodies.size(); ++first) {
    for (std::size_t second = first + 1; second < system.bodies.size();
         ++second) {
      const Body &first_body = system.bodies[first];
      const Body &second_body = system.bodies[second];

      const Vec3 displacement = second_body.position - first_body.position;

      const double softened_squared_distance =
          displacement.squared_norm() + softening * softening;

      if (softened_squared_distance == 0.0) {
        throw std::domain_error{
            "distinct bodies cannot occupy the same position "
            "when softening is zero"};
      }

      const double softened_distance = std::sqrt(softened_squared_distance);

      const double inverse_distance_cubed =
          1.0 / (softened_squared_distance * softened_distance);

      const Vec3 common =
          displacement * (gravitational_constant * inverse_distance_cubed);

      accelerations[first] += common * second_body.mass;
      accelerations[second] -= common * first_body.mass;
    }
  }

  return accelerations;
}

} // namespace orbitalforge::physics
