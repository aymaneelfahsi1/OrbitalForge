#include "orbitalforge/io/csv_writer.hpp"

#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace orbitalforge::io {

namespace {

[[nodiscard]] std::ofstream
open_output_file(const std::filesystem::path &path) {
  std::ofstream output{path};

  if (!output) {
    throw std::runtime_error{"failed to open output file: " + path.string()};
  }

  output << std::scientific << std::setprecision(17);

  return output;
}

void verify_output(const std::ofstream &output,
                   const std::filesystem::path &path) {
  if (!output) {
    throw std::runtime_error{"failed while writing output file: " +
                             path.string()};
  }
}

} // namespace

void write_diagnostics_csv(
    const std::filesystem::path &path,
    const std::vector<simulation::DiagnosticSample> &samples) {
  std::ofstream output = open_output_file(path);

  output << "step,"
         << "time_seconds,"
         << "total_energy,"
         << "relative_energy_drift,"
         << "momentum_x,"
         << "momentum_y,"
         << "momentum_z,"
         << "momentum_drift,"
         << "center_of_mass_x,"
         << "center_of_mass_y,"
         << "center_of_mass_z,"
         << "center_of_mass_drift\n";

  for (const simulation::DiagnosticSample &sample : samples) {
    output << sample.step << ',' << sample.simulation_time << ','
           << sample.total_energy << ',' << sample.relative_energy_drift << ','
           << sample.total_momentum.x() << ',' << sample.total_momentum.y()
           << ',' << sample.total_momentum.z() << ',' << sample.momentum_drift
           << ',' << sample.center_of_mass.x() << ','
           << sample.center_of_mass.y() << ',' << sample.center_of_mass.z()
           << ',' << sample.center_of_mass_drift << '\n';
  }

  verify_output(output, path);
}

void write_trajectory_csv(
    const std::filesystem::path &path,
    const std::vector<simulation::TrajectorySample> &samples) {
  std::ofstream output = open_output_file(path);

  output << "step,"
         << "time_seconds,"
         << "body_name,"
         << "mass,"
         << "position_x,"
         << "position_y,"
         << "position_z,"
         << "velocity_x,"
         << "velocity_y,"
         << "velocity_z\n";

  for (const simulation::TrajectorySample &sample : samples) {
    for (const physics::Body &body : sample.state.bodies) {
      output << sample.step << ',' << sample.simulation_time << ',' << body.name
             << ',' << body.mass << ',' << body.position.x() << ','
             << body.position.y() << ',' << body.position.z() << ','
             << body.velocity.x() << ',' << body.velocity.y() << ','
             << body.velocity.z() << '\n';
    }
  }

  verify_output(output, path);
}

} // namespace orbitalforge::io