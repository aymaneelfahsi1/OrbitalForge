#pragma once

#include <filesystem>

#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/simulation/simulation_runner.hpp"

namespace orbitalforge::io {

void write_simulation_outputs(
    const std::filesystem::path &output_directory,
    const app::Scenario &scenario,
    const simulation::SimulationConfig &config,
    const simulation::SimulationResult &result, double elapsed_milliseconds);

} // namespace orbitalforge::io
