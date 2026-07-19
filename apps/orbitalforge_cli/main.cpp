#include <algorithm>
#include <iomanip>
#include <ios>
#include <iostream>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/gravity.hpp"
#include "orbitalforge/physics/integrator.hpp"
#include "orbitalforge/physics/state.hpp"

namespace {

double specific_orbital_energy(const orbitalforge::physics::State &state,
                               double gravitational_parameter) {
  const double speed_squared = state.velocity.squared_norm();

  const double radius = state.position.norm();

  return (speed_squared / 2.0 - gravitational_parameter / radius);
};

} // namespace

int main() {
  using orbitalforge::math::Vec3;
  using orbitalforge::physics::Body;
  using orbitalforge::physics::gravitational_acceleration;
  using orbitalforge::physics::semi_implicit_euler_step;
  using orbitalforge::physics::State;

  constexpr double earth_mu = 3.986004418e14;
  constexpr double time_step = 1.0;
  constexpr double simulation_duration = 31536000;
  constexpr int output_interval = 600;

  Body satellite{"Explorer-1", 14.0, Vec3{7.0e6, 0.0, 0.0},
                 Vec3{0.0, 7546.0, 0.0}};

  double simulation_time = .0;

  const State initial_state = satellite.state();

  const double initial_radius = initial_state.position.norm();

  const double initial_energy =
      specific_orbital_energy(initial_state, earth_mu);

  double minimum_radius = initial_radius;
  double maximum_radius = initial_radius;

  std::cout << std::fixed << std::setprecision(3);

  std::cout << "Body: " << satellite.name << "\n\n";

  std::cout << "time (s)" << std::setw(18) << "radius (m)" << std::setw(18)
            << "speed (m/s)" << std::setw(22) << "energy (J/kg)" << '\n';

  for (int step = 0; simulation_time <= simulation_duration; ++step) {
    const State current_state = satellite.state();

    const double radius = current_state.position.norm();

    const double speed = current_state.velocity.norm();

    const double energy = specific_orbital_energy(current_state, earth_mu);

    minimum_radius = std::min(minimum_radius, radius);

    maximum_radius = std::max(maximum_radius, radius);

    if (step % output_interval == 0) {
      std::cout << std::setw(8) << simulation_time << std::setw(18) << radius
                << std::setw(18) << speed << std::setw(22) << energy << '\n';
    }

    const Vec3 acceleration =
        gravitational_acceleration(current_state.position, earth_mu);

    const State next_state =
        semi_implicit_euler_step(current_state, acceleration, time_step);

    satellite.set_state(next_state);

    simulation_time += time_step;
  }

  const State final_state = satellite.state();

  const double final_radius = satellite.position.norm();

  const double final_energy = specific_orbital_energy(final_state, earth_mu);

  std::cout << "\nSimulation summary\n";

  std::cout << "Body:           " << satellite.name << '\n';

  std::cout << "Mass:           " << satellite.mass << " kg\n";

  std::cout << "Initial radius: " << initial_radius << " m\n";

  std::cout << "Final radius:   " << final_radius << " m\n";

  std::cout << "Minimum radius: " << minimum_radius << " m\n";

  std::cout << "Maximum radius: " << maximum_radius << " m\n";

  std::cout << "Radius range:   " << maximum_radius - minimum_radius << " m\n";

  std::cout << "Initial energy: " << initial_energy << " J/kg\n";

  std::cout << "Final energy:   " << final_energy << " J/kg\n";

  std::cout << "Energy drift:   " << final_energy - initial_energy << " J/kg\n";

  return 0;

  return 0;
}