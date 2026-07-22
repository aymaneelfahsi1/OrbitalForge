#pragma once

#include <cstddef>
#include <vector>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"

namespace orbitalforge::simulation {

using orbitalforge::math::Vec3;
using orbitalforge::physics::SystemState;

struct SimulationConfig {
  double gravitational_constant;
  double time_step;
  std::size_t step_count;
  std::size_t output_interval;
  IntegratorKind integrator;
};

struct DiagnosticSample {
  std::size_t step;
  double simulation_time;
  double total_energy;
  double relative_energy_drift;
  Vec3 total_momentum;
  double momentum_drift;
  Vec3 center_of_mass;
  double center_of_mass_drift;
};

struct SimulationResult {
  SystemState final_system_state;
  std::vector<DiagnosticSample> diagnostics;
};

class SimulationRunner {

public:
  [[nodiscard]] SimulationResult run(const SystemState &initial_state,
                                     const SimulationConfig &config) const;

private:
  static void validate(const SimulationConfig &config);
};

} // namespace orbitalforge::simulation
