#include "orbitalforge/simulation/step.hpp"

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/gravity.hpp"
#include "orbitalforge/physics/integrator.hpp"
#include "orbitalforge/physics/state.hpp"
#include "orbitalforge/physics/system_state.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace orbitalforge::simulation {

using math::Vec3;
using physics::Body;
using physics::explicit_euler_step;
using physics::gravitational_accelerations;
using physics::semi_implicit_euler_step;
using physics::State;
using physics::SystemState;
using std::vector;

// anonymous namespace, everything inside it is private to step.cpp file. it
// holds implemntation helpers
namespace {

void validate_time_step(double time_step) {
  if (time_step <= 0.0) {
    throw std::invalid_argument{"time_step must be strictly positive"};
  }
}

void validate_acceleration_count(const SystemState &system,
                                 const vector<Vec3> &accelerations) {
  if (accelerations.size() != system.bodies.size()) {
    throw std::logic_error{"each body must have exactly one acceleration"};
  }
}

template <typename Integrator>
void advance_system_with_fixed_acceleration(SystemState &system,
                                            double gravitational_constant,
                                            double time_step,
                                            const Integrator &integrator) {
  validate_time_step(time_step);
  const auto accelerations =
      gravitational_accelerations(system, gravitational_constant);

  validate_acceleration_count(system, accelerations);

  for (std::size_t index = 0; index < system.bodies.size(); ++index) {
    Body &body = system.bodies[index];

    const State current_state = body.state();

    const State next_state =
        integrator(current_state, accelerations[index], time_step);

    body.set_state(next_state);
  }
}

struct SystemDerivative {
  vector<Vec3> position_rates;
  vector<Vec3> velocity_rates;
};

[[nodiscard]] SystemDerivative
evaluate_derivative(const SystemState &system, double gravitational_constant) {
  vector<Vec3> accelerations =
      gravitational_accelerations(system, gravitational_constant);

  validate_acceleration_count(system, accelerations);

  vector<Vec3> velocities;
  velocities.reserve(system.bodies.size());

  for (const Body &body : system.bodies) {
    velocities.push_back(body.velocity);
  }

  return SystemDerivative{std::move(velocities), std::move(accelerations)};
}

[[nodiscard]] SystemState offset_system(const SystemState &base_system,
                                        const SystemDerivative &derivative,
                                        double scale) {

  if (derivative.position_rates.size() != base_system.bodies.size() ||
      derivative.velocity_rates.size() != base_system.bodies.size()) {
    throw std::logic_error{"system derivative size must match body count"};
  }

  SystemState result = base_system;

  for (std::size_t index = 0; index < result.bodies.size(); ++index) {

    const State base_state = base_system.bodies[index].state();

    result.bodies[index].set_state(
        State{base_state.position + derivative.position_rates[index] * scale,
              base_state.velocity + derivative.velocity_rates[index] * scale});
  }

  return result;
}

} // namespace

void advance_explicit_euler_step(physics::SystemState &system,
                                 double gravitational_constant,
                                 double time_step) {

  advance_system_with_fixed_acceleration(
      system, gravitational_constant, time_step,
      [](const State &state, const Vec3 &acceleration, double dt) {
        return explicit_euler_step(state, acceleration, dt);
      });
}

void advance_semi_implicit_euler_step(physics::SystemState &system,
                                      double gravitational_constant,
                                      double time_step) {

  advance_system_with_fixed_acceleration(
      system, gravitational_constant, time_step,
      [](const State &state, const Vec3 &acceleration, double dt) {
        return semi_implicit_euler_step(state, acceleration, dt);
      });
}

void advance_velocity_verlet_step(physics::SystemState &system,
                                  double gravitational_constant,
                                  double time_step) {
  validate_time_step(time_step);

  const vector<Vec3> initial_accelerations =
      gravitational_accelerations(system, gravitational_constant);

  validate_acceleration_count(system, initial_accelerations);

  const double half_time_step_squared = .5 * time_step * time_step;

  for (std::size_t index = 0; index < system.bodies.size(); index++) {
    Body &body = system.bodies[index];

    Vec3 position_advanced =
        body.position + body.velocity * time_step +
        initial_accelerations[index] * half_time_step_squared;

    body.set_state(State{position_advanced, body.velocity});
  }

  const vector<Vec3> final_accelerations =
      gravitational_accelerations(system, gravitational_constant);

  validate_acceleration_count(system, final_accelerations);

  const double half_time_step = .5 * time_step;

  for (std::size_t index = 0; index < system.bodies.size(); index++) {
    Body &body = system.bodies[index];

    Vec3 velocity_advanced = body.velocity + (initial_accelerations[index] +
                                              final_accelerations[index]) *
                                                 half_time_step;

    body.set_state(State{body.position, velocity_advanced});
  }
}

void advance_leapfrog_step(SystemState &system, double gravitational_constant,
                           double time_step) {
  validate_time_step(time_step);

  const vector<Vec3> initial_accelerations =
      gravitational_accelerations(system, gravitational_constant);

  validate_acceleration_count(system, , initial_accelerations);
}

} // namespace orbitalforge::simulation