#include "orbitalforge/io/simulation_output_writer.hpp"

#include <filesystem>

#include "orbitalforge/io/csv_writer.hpp"
#include "orbitalforge/io/metadata_writer.hpp"

namespace orbitalforge::io {

void write_simulation_outputs(
    const std::filesystem::path &output_directory,
    const app::Scenario &scenario,
    const simulation::SimulationConfig &config,
    const simulation::SimulationResult &result, double elapsed_milliseconds) {
  std::filesystem::create_directories(output_directory);
  write_trajectory_csv(output_directory / "trajectory.csv", result.trajectory);
  write_diagnostics_csv(output_directory / "diagnostics.csv",
                        result.diagnostics);
  write_metadata(output_directory / "metadata.txt", scenario, config, result,
                 elapsed_milliseconds);
}

} // namespace orbitalforge::io
