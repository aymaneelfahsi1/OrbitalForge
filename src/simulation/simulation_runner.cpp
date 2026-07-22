#include "orbitalforge/simulation/simulation_runner.hpp"
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>

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

[[nodiscard]] DiagnosticSample
make_sample(const SystemState &state, std::size_t step, double time_step,
            double gravitational_constant, double initial_energy,
            const Vec3 &initial_momentum, const Vec3 &initial_center_of_mass) {

  const double current_energy = total_energy(state, gravitational_constant);
  const Vec3 current_momentum = total_momentum(state);
  const Vec3 current_center_of_mass = center_of_mass(state);

  return DiagnosticSample{
      .step = step,
      .simulation_time = static_cast<double>(step) * time_step,
      .total_energy = current_energy,
      .relative_energy_drift = relative_drift(initial_energy, current_energy),
      .total_momentum = current_momentum,
      .momentum_drift = (initial_momentum - current_momentum).norm(),
      .center_of_mass = current_center_of_mass,
      .center_of_mass_drift =
          (current_center_of_mass - initial_center_of_mass).norm()

  };
}

} // namespace

void SimulationRunner::validate(const SimulationConfig &config) {

  if (!std::isfinite(config.gravitational_constant) ||
      config.gravitational_constant <= 0.0) {
    throw std::invalid_argument{
        "gravitational constant must be postive and finite"};
  }

  if (config.output_interval == 0) {
    throw std::invalid_argument{"output interval must be greater than zero"};
  }
}

SimulationResult SimulationRunner::run(const SystemState &initial_state,
                                       const SimulationConfig &config) const {

  validate(config);

  SystemState state = initial_state;
  StepFunction *const advance = step_function(config.integrator);

  const double initial_energy =
      total_energy(state, config.gravitational_constant);

  const Vec3 initial_momentum = total_momentum(state);
  const Vec3 initial_center_of_mass = center_of_mass(state);

  std::vector<DiagnosticSample> samples;

  const std::size_t expected_samples =
      config.step_count / config.output_interval + 2;

  samples.reserve(expected_samples);

  samples.push_back(make_sample(state, 0, config.time_step,
                                config.gravitational_constant, initial_energy,
                                initial_momentum, initial_center_of_mass));

  for (std::size_t step = 1; step <= config.step_count; ++step) {
    advance(state, config.gravitational_constant, config.time_step);

    const bool output_step = step % config.output_interval == 0;
    const bool final_step = step == config.step_count;

    if (output_step || final_step) {
      samples.push_back(make_sample(
          state, step, config.time_step, config.gravitational_constant,
          initial_energy, initial_momentum, initial_center_of_mass));
    }
  }

  return SimulationResult{.final_system_state = std::move(state),
                          .diagnostics = std::move(samples)};
}

} // namespace orbitalforge::simulation