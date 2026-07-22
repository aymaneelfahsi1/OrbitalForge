#include "orbitalforge/physics/diagnostics.hpp"
#include "orbitalforge/math/vec3.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace orbitalforge::physics {

using math::Vec3;

double kinetic_energy(const SystemState &system) noexcept {
  double total{};

  for (const Body &body : system.bodies) {
    total += 0.5 * body.mass * body.velocity.squared_norm();
  }

  return total;
}

double potential_energy(const SystemState &system,
                        double gravitational_constant, double softening) {

  if (!std::isfinite(gravitational_constant) || gravitational_constant <= 0.0) {
    throw std::invalid_argument{"gravitational constant must be positive"};
  }

  if (!std::isfinite(softening) || softening < 0.0) {
    throw std::invalid_argument{"softening must be non-negative and finite"};
  }

  double total_energy{};

  for (std::size_t first_index = 0; first_index < system.bodies.size();
       ++first_index) {

    for (std::size_t second_index = first_index + 1;
         second_index < system.bodies.size(); ++second_index) {
      const Body &first = system.bodies[first_index];
      const Body &second = system.bodies[second_index];

      const Vec3 displacement = second.position - first.position;

      const double softened_distance =
          std::sqrt(displacement.squared_norm() + softening * softening);

      if (softened_distance == 0.0) {
        throw std::domain_error{
            "potential energy is undefined for coincident bodies "
            "when softening is zero"};
      }

      total_energy -=
          gravitational_constant * first.mass * second.mass / softened_distance;
    }
  }
  return total_energy;
}

double total_energy(const SystemState &system, double gravitational_constant,
                    double softening) {
  return kinetic_energy(system) +
         potential_energy(system, gravitational_constant, softening);
}

Vec3 total_momentum(const SystemState &system) noexcept {

  Vec3 momentum{};

  for (const Body &body : system.bodies) {
    momentum += body.mass * body.velocity;
  }

  return momentum;
}

Vec3 center_of_mass(const SystemState &system) {
  Vec3 weighted_position{};

  double total_mass{};

  for (const Body &body : system.bodies) {
    weighted_position += body.mass * body.position;
    total_mass += body.mass;
  }

  if (total_mass == 0.0) {
    throw std::domain_error{"center of mass is undefined for an empty system"};
  }

  return weighted_position / total_mass;
}

} // namespace orbitalforge::physics
