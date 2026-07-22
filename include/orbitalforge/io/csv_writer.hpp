#pragma once

#include <filesystem>
#include <vector>

#include "orbitalforge/simulation/simulation_runner.hpp"

namespace orbitalforge::io {

void write_diagnostics_csv(
    const std::filesystem::path &path,
    const std::vector<simulation::DiagnosticSample> &samples);

void write_trajectory_csv(
    const std::filesystem::path &path,
    const std::vector<simulation::TrajectorySample> &samples);

} // namespace orbitalforge::io