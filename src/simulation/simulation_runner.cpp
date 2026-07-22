#include "orbitalforge/simulation/simulation_runner.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/diagnostics.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"

namespace orbitalforge::simulation {

using orbitalforge::math::Vec3;
using orbitalforge::physics::center_of_mass;
using orbitalforge::physics::SystemState;
using orbitalforge::physics::total_energy;
using orbitalforge::physics::total_momentum;

namespace {

[[nodiscard]] double relative_drift(double initial_value,
                                    double current_value) {
  const double absolute_drift = std::abs(current_value - initial_value);

  if (initial_value == 0.0) {
    return absolute_drift;
  }

  return absolute_drift / std::abs(initial_value);
}

[[nodiscard]] DiagnosticSample make_diagnostic_sample(
    const SystemState &state, std::size_t step, double time_step,
    double gravitational_constant, double softening, double initial_energy,
    const Vec3 &initial_momentum, const Vec3 &initial_center_of_mass) {
  const double current_energy =
      total_energy(state, gravitational_constant, softening);

  const Vec3 current_momentum = total_momentum(state);

  const Vec3 current_center_of_mass = center_of_mass(state);

  return DiagnosticSample{
      .step = step,
      .simulation_time = static_cast<double>(step) * time_step,
      .total_energy = current_energy,
      .relative_energy_drift = relative_drift(initial_energy, current_energy),
      .total_momentum = current_momentum,
      .momentum_drift = (current_momentum - initial_momentum).norm(),
      .center_of_mass = current_center_of_mass,
      .center_of_mass_drift =
          (current_center_of_mass - initial_center_of_mass).norm(),
  };
}

[[nodiscard]] TrajectorySample make_trajectory_sample(const SystemState &state,
                                                      std::size_t step,
                                                      double simulation_time) {
  return TrajectorySample{
      .step = step, .simulation_time = simulation_time, .state = state};
}

} // namespace

void SimulationRunner::validate(const SystemState &initial_state,
                                const SimulationConfig &config) {
  if (initial_state.bodies.empty()) {
    throw std::invalid_argument{
        "initial system must contain at least one body"};
  }

  if (!std::isfinite(config.gravitational_constant) ||
      config.gravitational_constant <= 0.0) {
    throw std::invalid_argument{
        "gravitational constant must be positive and finite"};
  }

  if (!std::isfinite(config.softening) || config.softening < 0.0) {
    throw std::invalid_argument{"softening must be non-negative and finite"};
  }

  if (!std::isfinite(config.time_step) || config.time_step <= 0.0) {
    throw std::invalid_argument{"time step must be positive and finite"};
  }

  if (config.output_interval == 0) {
    throw std::invalid_argument{"output interval must be greater than zero"};
  }

  if (step_function(config.integrator) == nullptr) {
    throw std::invalid_argument{"unknown integrator"};
  }
}

SimulationResult SimulationRunner::run(const SystemState &initial_state,
                                       const SimulationConfig &config) const {
  validate(initial_state, config);

  SystemState state = initial_state;

  StepFunction *const advance = step_function(config.integrator);

  const double initial_energy =
      total_energy(state, config.gravitational_constant, config.softening);

  const Vec3 initial_momentum = total_momentum(state);

  const Vec3 initial_center_of_mass = center_of_mass(state);

  const std::size_t interval_samples =
      config.step_count / config.output_interval;

  const bool final_step_needs_sample =
      config.step_count % config.output_interval != 0;

  const std::size_t expected_samples_count =
      1 + interval_samples + static_cast<std::size_t>(final_step_needs_sample);

  std::vector<DiagnosticSample> diagnostics;
  diagnostics.reserve(expected_samples_count);

  std::vector<TrajectorySample> trajectory;
  trajectory.reserve(expected_samples_count);

  diagnostics.push_back(make_diagnostic_sample(
      state, 0, config.time_step, config.gravitational_constant,
      config.softening, initial_energy, initial_momentum,
      initial_center_of_mass));
  trajectory.push_back(make_trajectory_sample(state, 0, 0.0));

  for (std::size_t step = 1; step <= config.step_count; ++step) {
    advance(state, config.gravitational_constant, config.softening,
            config.time_step);

    const bool output_step = step % config.output_interval == 0;

    const bool final_step = step == config.step_count;

    const double simulation_time = static_cast<double>(step) * config.time_step;

    if (output_step || final_step) {
      diagnostics.push_back(make_diagnostic_sample(
          state, step, config.time_step, config.gravitational_constant,
          config.softening, initial_energy, initial_momentum,
          initial_center_of_mass));

      trajectory.push_back(
          make_trajectory_sample(state, step, simulation_time));
    }
  }

  return SimulationResult{.final_system_state = std::move(state),
                          .diagnostics = std::move(diagnostics),
                          .trajectory = std::move(trajectory)};
}

} // namespace orbitalforge::simulation
