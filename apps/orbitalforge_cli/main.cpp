#include <chrono>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <variant>

#include "orbitalforge/app/parse_error.hpp"
#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/app/scenario_parser.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"
#include "orbitalforge/simulation/simulation_runner.hpp"

namespace {

using orbitalforge::app::parse_scenario_file;
using orbitalforge::app::ParseError;
using orbitalforge::app::Scenario;
using orbitalforge::app::ScenarioParseResult;
using orbitalforge::simulation::DiagnosticSample;
using orbitalforge::simulation::integrator_name;
using orbitalforge::simulation::SimulationConfig;
using orbitalforge::simulation::SimulationResult;
using orbitalforge::simulation::SimulationRunner;

constexpr int success_exit_code = 0;
constexpr int usage_error_exit_code = 1;
constexpr int scenario_error_exit_code = 2;
constexpr int simulation_error_exit_code = 3;

void print_usage(std::string_view executable_name) {
  std::cerr << "Usage:\n"
            << "  " << executable_name << " simulate <scenario.orbit>\n\n"
            << "Example:\n"
            << "  " << executable_name
            << " simulate scenarios/two_body.orbit\n";
}

[[nodiscard]] SimulationConfig
make_simulation_config(const Scenario &scenario) {
  return SimulationConfig{
      .gravitational_constant = scenario.gravitational_constant,
      .softening = scenario.softening,
      .time_step = scenario.time_step,
      .step_count = scenario.step_count,
      .output_interval = scenario.output_interval,
      .integrator = scenario.integrator,
  };
}

void print_scenario_summary(const Scenario &scenario,
                            const SimulationConfig &config) {
  const double simulated_duration =
      config.time_step * static_cast<double>(config.step_count);

  std::cout << "OrbitalForge\n"
            << "============\n\n"
            << "Scenario:               " << scenario.name << '\n'
            << "Bodies:                 "
            << scenario.initial_state.bodies.size() << '\n'
            << "Integrator:             " << integrator_name(config.integrator)
            << '\n'
            << "Gravitational constant: " << std::scientific
            << std::setprecision(10) << config.gravitational_constant << '\n'
            << "Softening:              " << config.softening << '\n'
            << "Time step:              " << config.time_step << '\n'
            << "Step count:             " << config.step_count << '\n'
            << "Output interval:        " << config.output_interval << '\n'
            << "Simulated duration:     " << simulated_duration << '\n'
            << "Seed:                   " << scenario.seed << "\n\n";
}

void print_diagnostics_header() {
  std::cout << std::right << std::setw(10) << "Step" << std::setw(18) << "Time"
            << std::setw(22) << "Energy" << std::setw(20) << "Rel E drift"
            << std::setw(20) << "Momentum drift" << std::setw(20) << "COM drift"
            << '\n';

  std::cout << std::string(110, '-') << '\n';
}

void print_diagnostic_sample(const DiagnosticSample &sample) {
  std::cout << std::right << std::setw(10) << sample.step

            << std::setw(18) << std::fixed << std::setprecision(6)
            << sample.simulation_time

            << std::setw(22) << std::scientific << std::setprecision(8)
            << sample.total_energy

            << std::setw(20) << sample.relative_energy_drift

            << std::setw(20) << sample.momentum_drift

            << std::setw(20) << sample.center_of_mass_drift

            << '\n';
}

void print_final_summary(const SimulationResult &result,
                         double elapsed_milliseconds) {
  const DiagnosticSample &initial_sample = result.diagnostics.front();

  const DiagnosticSample &final_sample = result.diagnostics.back();

  std::cout << "\nSimulation complete\n"
            << "===================\n"
            << std::scientific << std::setprecision(10)
            << "Initial energy:         " << initial_sample.total_energy << '\n'
            << "Final energy:           " << final_sample.total_energy << '\n'
            << "Relative energy drift:  " << final_sample.relative_energy_drift
            << '\n'
            << "Momentum drift:         " << final_sample.momentum_drift << '\n'
            << "Center-of-mass drift:   " << final_sample.center_of_mass_drift
            << '\n'
            << std::fixed << std::setprecision(3)
            << "Execution time:         " << elapsed_milliseconds << " ms\n";
}

void print_parse_error(const std::string &scenario_path,
                       const ParseError &error) {
  std::cerr << "Failed to load scenario '" << scenario_path << "'";

  if (error.line_number != 0) {
    std::cerr << " at line " << error.line_number;
  }

  std::cerr << ": " << error.message << '\n';
}

[[nodiscard]] int run_simulate_command(const std::string &scenario_path) {
  const ScenarioParseResult parse_result = parse_scenario_file(scenario_path);

  if (const auto *error = std::get_if<ParseError>(&parse_result)) {
    print_parse_error(scenario_path, *error);
    return scenario_error_exit_code;
  }

  const Scenario &scenario = std::get<Scenario>(parse_result);

  const SimulationConfig config = make_simulation_config(scenario);

  print_scenario_summary(scenario, config);
  print_diagnostics_header();

  const SimulationRunner runner;

  const auto start_time = std::chrono::steady_clock::now();

  const SimulationResult result = runner.run(scenario.initial_state, config);

  const auto end_time = std::chrono::steady_clock::now();

  const std::chrono::duration<double, std::milli> elapsed_time =
      end_time - start_time;

  for (const DiagnosticSample &sample : result.diagnostics) {
    print_diagnostic_sample(sample);
  }

  print_final_summary(result, elapsed_time.count());

  return success_exit_code;
}

} // namespace

int main(int argc, char *argv[]) {
  if (argc != 3) {
    print_usage(argv[0]);
    return usage_error_exit_code;
  }

  const std::string_view command{argv[1]};

  if (command != "simulate") {
    std::cerr << "Unknown command: " << command << "\n\n";

    print_usage(argv[0]);

    return usage_error_exit_code;
  }

  try {
    return run_simulate_command(argv[2]);
  } catch (const std::exception &error) {
    std::cerr << "Simulation failed: " << error.what() << '\n';

    return simulation_error_exit_code;
  }
}