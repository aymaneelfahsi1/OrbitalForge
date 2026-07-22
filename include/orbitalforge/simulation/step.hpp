#pragma once

#include "orbitalforge/physics/system_state.hpp"

namespace orbitalforge::simulation {

void advance_explicit_euler_step(physics::SystemState &system,
                                 double gravitational_constant,
                                 double softening, double time_step);

void advance_semi_implicit_euler_step(physics::SystemState &system,
                                      double gravitational_constant,
                                      double softening, double time_step);

void advance_velocity_verlet_step(physics::SystemState &system,
                                  double gravitational_constant,
                                  double softening, double time_step);

void advance_leapfrog_step(physics::SystemState &system,
                           double gravitational_constant, double softening,
                           double time_step);

void advance_runge_kutta_4_step(physics::SystemState &system,
                                double gravitational_constant, double softening,
                                double time_step);

inline void advance_explicit_euler_step(physics::SystemState &system,
                                        double gravitational_constant,
                                        double time_step) {
  advance_explicit_euler_step(system, gravitational_constant, 0.0, time_step);
}

inline void advance_semi_implicit_euler_step(
    physics::SystemState &system, double gravitational_constant,
    double time_step) {
  advance_semi_implicit_euler_step(system, gravitational_constant, 0.0,
                                   time_step);
}

inline void advance_velocity_verlet_step(physics::SystemState &system,
                                         double gravitational_constant,
                                         double time_step) {
  advance_velocity_verlet_step(system, gravitational_constant, 0.0, time_step);
}

inline void advance_leapfrog_step(physics::SystemState &system,
                                  double gravitational_constant,
                                  double time_step) {
  advance_leapfrog_step(system, gravitational_constant, 0.0, time_step);
}

inline void advance_runge_kutta_4_step(physics::SystemState &system,
                                       double gravitational_constant,
                                       double time_step) {
  advance_runge_kutta_4_step(system, gravitational_constant, 0.0, time_step);
}

} // namespace orbitalforge::simulation
