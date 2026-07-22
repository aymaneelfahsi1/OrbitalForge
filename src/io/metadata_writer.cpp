#include "orbitalforge/io/metadata_writer.hpp"

#include <fstream>
#include <iomanip>
#include <stdexcept>

#include "orbitalforge/simulation/integrator_kind.hpp"

namespace orbitalforge::io {

void write_metadata(const std::filesystem::path &path,
                    const app::Scenario &scenario,
                    const simulation::SimulationConfig &config,
                    const simulation::SimulationResult &result,
                    double elapsed_milliseconds) {
  std::ofstream output{path};

  if (!output) {
    throw std::runtime_error{"failed to open metadata file: " + path.string()};
  }

  output << std::scientific << std::setprecision(17);

  output << "application=OrbitalForge\n"
         << "scenario=" << scenario.name << '\n'
         << "body_count=" << scenario.initial_state.bodies.size() << '\n'
         << "integrator=" << simulation::integrator_name(config.integrator)
         << '\n'
         << "gravitational_constant=" << config.gravitational_constant << '\n'
         << "softening=" << config.softening << '\n'
         << "time_step=" << config.time_step << '\n'
         << "step_count=" << config.step_count << '\n'
         << "output_interval=" << config.output_interval << '\n'
         << "seed=" << scenario.seed << '\n'
         << "trajectory_sample_count=" << result.trajectory.size() << '\n'
         << "diagnostic_sample_count=" << result.diagnostics.size() << '\n'
         << "elapsed_milliseconds=" << elapsed_milliseconds << '\n';

#ifdef NDEBUG
  output << "build_type=release\n";
#else
  output << "build_type=debug\n";
#endif

  if (!result.diagnostics.empty()) {
    const simulation::DiagnosticSample &initial = result.diagnostics.front();

    const simulation::DiagnosticSample &final = result.diagnostics.back();

    output << "initial_energy=" << initial.total_energy << '\n'
           << "final_energy=" << final.total_energy << '\n'
           << "relative_energy_drift=" << final.relative_energy_drift << '\n'
           << "momentum_drift=" << final.momentum_drift << '\n'
           << "center_of_mass_drift=" << final.center_of_mass_drift << '\n';
  }

  if (!output) {
    throw std::runtime_error{"failed while writing metadata file: " +
                             path.string()};
  }
}

} // namespace orbitalforge::io