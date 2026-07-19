#include <iostream>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/gravity.hpp"
#include "orbitalforge/physics/integrator.hpp"
#include "orbitalforge/physics/state.hpp"

int main() {
  using orbitalforge::math::Vec3;
  using orbitalforge::physics::Body;
  using orbitalforge::physics::gravitational_acceleration;
  using orbitalforge::physics::semi_implicit_euler_step;
  using orbitalforge::physics::State;

  constexpr double earth_mu = 3.986004418e14;
  constexpr double time_step = 1.0;

  const Body satellite{"Explorer-1", 14.0, Vec3{7.0e6, 0.0, 0.0},
                       Vec3{0.0, 7546.0, 0.0}};

  const State current_state{satellite.position, satellite.velocity};

  const Vec3 acceleration =
      gravitational_acceleration(current_state.position, earth_mu);

  const State next_state =
      semi_implicit_euler_step(current_state, acceleration, time_step);

  std::cout << "OrbitalForge\n\n";

  std::cout << "Body: " << satellite.name << '\n';

  std::cout << "Current position: " << current_state.position << " m\n";

  std::cout << "Current velocity: " << current_state.velocity << " m/s\n";

  std::cout << "Acceleration: " << acceleration << " m/s^2\n";

  std::cout << "Next position: " << next_state.position << " m\n";

  std::cout << "Next velocity: " << next_state.velocity << " m/s\n";

  return 0;
}