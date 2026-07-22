#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"

namespace orbitalforge::app {

struct Scenario {
  std::string name;

  double gravitational_constant{};
  double softening{};
  double time_step{};

  std::size_t step_count{};
  std::size_t output_interval{};

  simulation::IntegratorKind integrator{
      simulation::IntegratorKind::velocity_verlet};

  std::uint64_t seed{};

  physics::SystemState initial_state;
};

void validate_scenario(const Scenario &scenario);

} // namespace orbitalforge::app