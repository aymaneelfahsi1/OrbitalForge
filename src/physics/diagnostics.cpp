#include "orbitalforge/physics/diagnostics.hpp"
#include "orbitalforge/math/vec3.hpp"

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
                        double gravitational_constant) {

  if (gravitational_constant <= 0.0) {
    throw std::invalid_argument{"gravitational constant must be positive"};
  }

  double total{};

  for (std::size_t first_index = 0; first_index < system.bodies.size();
       ++first_index) {

    for (std::size_t second_index = first_index + 1;
         second_index < system.bodies.size(); ++second_index) {
      const Body &first = system.bodies[first_index];
      const Body &second = system.bodies[second_index];

      const Vec3 displacement = second.position - first.position;

      const double distance = displacement.norm();

      if (distance == 0) {
        throw std::domain_error{
            "potential energy is undefined for coincident bodies"};
      }

      total -= gravitational_constant * first.mass * second.mass / distance;
    }
  }
  return total;
}

double total_energy(const SystemState &system, double gravitational_constant) {
  return kinetic_energy(system) +
         potential_energy(system, gravitational_constant);
}

Vec3 total_momentum(const SystemState &system) noexcept {

  Vec3 momentum{};

  for (const Body &body : system.bodies) {
    momentum += body.mass * body.velocity;
  }

  return momentum;
}

Vec3 center_of_mass(const SystemState &system) {
  Vec3 weighted_postion{};

  double total_mass{};

  for (const Body &body : system.bodies) {
    weighted_postion += body.mass * body.position;
    total_mass += body.mass;
  }

  if (total_mass == 0.0) {
    throw std::domain_error{"center of mass is undefined for an empty system"};
  }

  return weighted_postion / total_mass;
}

} // namespace orbitalforge::physics