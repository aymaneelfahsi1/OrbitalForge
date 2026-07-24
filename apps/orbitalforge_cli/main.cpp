#include <array>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "orbitalforge/app/benchmark.hpp"
#include "orbitalforge/app/parse_error.hpp"
#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/app/scenario_parser.hpp"
#include "orbitalforge/io/simulation_output_writer.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"
#include "orbitalforge/simulation/simulation_runner.hpp"

namespace {

using orbitalforge::app::parse_benchmark_body_counts;
using orbitalforge::app::benchmark_all_integrators;
using orbitalforge::app::IntegratorBenchmarkResult;
using orbitalforge::app::BenchmarkSortKind;
using orbitalforge::app::parse_scenario_file;
using orbitalforge::app::ParseError;
using orbitalforge::app::Scenario;
using orbitalforge::app::ScenarioParseResult;
using orbitalforge::app::sort_integrator_results;
using orbitalforge::app::parse_benchmark_sort_kind;
using orbitalforge::app::apply_benchmark_sort;
using orbitalforge::io::write_simulation_outputs;
using orbitalforge::simulation::DiagnosticSample;
using orbitalforge::simulation::integrator_name;
using orbitalforge::simulation::IntegratorKind;
using orbitalforge::simulation::SimulationConfig;
using orbitalforge::simulation::SimulationResult;
using orbitalforge::simulation::SimulationRunner;

constexpr int success_exit_code = 0;
constexpr int usage_error_exit_code = 1;
constexpr int simulation_error_exit_code = 3;

constexpr std::size_t default_warmup_count = 2;
constexpr std::size_t default_repetition_count = 5;

struct SimulateArguments {
  std::filesystem::path scenario_path;
  std::filesystem::path output_directory;
};

struct CompareResult {
  IntegratorKind integrator;
  double final_energy;
  double relative_energy_drift;
  double momentum_drift;
  double center_of_mass_drift;
  double elapsed_milliseconds;
};

struct BenchmarkArguments {
  std::filesystem::path scenario_path;
  std::vector<std::size_t> body_counts;
  BenchmarkSortKind sort_kind;
};

constexpr std::array integrators{
    IntegratorKind::explicit_euler,  IntegratorKind::semi_implicit_euler,
    IntegratorKind::velocity_verlet, IntegratorKind::leapfrog,
    IntegratorKind::runge_kutta_4,
};

void print_usage(std::string_view executable_name) {
  std::cerr << "Usage:\n"
            << "  " << executable_name
            << " simulate <scenario.orbit> [--output <directory>]\n"
            << "  " << executable_name << " compare <scenario.orbit>\n"
            << "  " << executable_name
            << " benchmark <scenario.orbit> --sizes <list>"
               " [--sort <sort|stable-sort|partial-sort|nth-element|partition>]\n\n"
            << "Examples:\n"
            << "  " << executable_name << " simulate scenarios/two_body.orbit\n"
            << "  " << executable_name
            << " simulate scenarios/solar_system.orbit "
            << "--output runs/solar-system\n";
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

[[nodiscard]] SimulateArguments parse_simulate_arguments(int argc,
                                                         char *argv[]) {
  if (argc != 3 && argc != 5) {
    throw std::invalid_argument{
        "simulate expects a scenario path and optional --output directory"};
  }

  SimulateArguments arguments{
      .scenario_path = argv[2],
      .output_directory = "runs/latest",
  };

  if (argc == 5) {
    const std::string_view option{argv[3]};

    if (option != "--output") {
      throw std::invalid_argument{"unknown simulate option: " +
                                  std::string{option}};
    }

    arguments.output_directory = argv[4];

    if (arguments.output_directory.empty()) {
      throw std::invalid_argument{"output directory must not be empty"};
    }
  }

  return arguments;
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
                         double elapsed_milliseconds,
                         const std::filesystem::path &output_directory) {
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
            << "Execution time:         " << elapsed_milliseconds << " ms\n"
            << "Output directory:       " << output_directory << '\n';
}

void print_parse_error(const std::filesystem::path &scenario_path,
                       const ParseError &error) {
  std::cerr << "Failed to load scenario '" << scenario_path.string() << "'";

  if (error.line_number != 0) {
    std::cerr << " at line " << error.line_number;
  }

  std::cerr << ": " << error.message << '\n';
}

[[nodiscard]] Scenario
load_scenario(const std::filesystem::path &scenario_path) {
  ScenarioParseResult parse_result = parse_scenario_file(scenario_path);

  if (const auto *error = std::get_if<ParseError>(&parse_result)) {
    print_parse_error(scenario_path, *error);

    throw std::runtime_error{"scenario could not be loaded"};
  }

  return std::get<Scenario>(std::move(parse_result));
}

[[nodiscard]] CompareResult run_comparison(const Scenario &scenario,
                                           IntegratorKind integrator) {
  SimulationConfig config = make_simulation_config(scenario);

  config.integrator = integrator;

  const SimulationRunner runner;

  const auto start_time = std::chrono::steady_clock::now();

  const SimulationResult result = runner.run(scenario.initial_state, config);

  const auto end_time = std::chrono::steady_clock::now();

  const std::chrono::duration<double, std::milli> elapsed_time =
      end_time - start_time;

  const DiagnosticSample &final_sample = result.diagnostics.back();

  return CompareResult{
      .integrator = integrator,
      .final_energy = final_sample.total_energy,
      .relative_energy_drift = final_sample.relative_energy_drift,
      .momentum_drift = final_sample.momentum_drift,
      .center_of_mass_drift = final_sample.center_of_mass_drift,
      .elapsed_milliseconds = elapsed_time.count(),
  };
}

[[nodiscard]] int run_simulate_command(const SimulateArguments &arguments) {
  const Scenario scenario = load_scenario(arguments.scenario_path);

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

  write_simulation_outputs(arguments.output_directory, scenario, config, result,
                           elapsed_time.count());

  print_final_summary(result, elapsed_time.count(), arguments.output_directory);

  return success_exit_code;
}

void print_compare_header(const Scenario &scenario) {
  std::cout << "OrbitalForge integrator comparison\n"
            << "==================================\n\n"
            << "Scenario:        " << scenario.name << '\n'
            << "Bodies:          " << scenario.initial_state.bodies.size()
            << '\n'
            << "Time step:       " << scenario.time_step << '\n'
            << "Step count:      " << scenario.step_count << '\n'
            << "Softening:       " << scenario.softening << "\n\n"

            << std::left << std::setw(24) << "Integrator"

            << std::right << std::setw(22) << "Final energy"

            << std::setw(20) << "Rel E drift"

            << std::setw(20) << "Momentum drift"

            << std::setw(20) << "COM drift"

            << std::setw(16) << "Time (ms)"

            << '\n';

  std::cout << std::string(122, '-') << '\n';
}

void print_compare_result(const CompareResult &result) {
  std::cout << std::left << std::setw(24) << integrator_name(result.integrator)

            << std::right << std::scientific << std::setprecision(8)

            << std::setw(22) << result.final_energy

            << std::setw(20) << result.relative_energy_drift

            << std::setw(20) << result.momentum_drift

            << std::setw(20) << result.center_of_mass_drift

            << std::fixed << std::setprecision(3)

            << std::setw(16) << result.elapsed_milliseconds

            << '\n';
}

[[nodiscard]] int
run_compare_command(const std::filesystem::path &scenario_path) {
  const Scenario scenario = load_scenario(scenario_path);

  print_compare_header(scenario);

  for (const IntegratorKind integrator : integrators) {
    const CompareResult result = run_comparison(scenario, integrator);

    print_compare_result(result);
  }

  return success_exit_code;
}

[[nodiscard]] BenchmarkArguments parse_benchmark_arguments(int argc,
                                                           char *argv[]) {
  if (argc != 5 && argc != 7) {
    throw std::invalid_argument{
        "benchmark expects a scenario path and --sizes list"};
  }

  const std::string_view option{argv[3]};

  if (option != "--sizes") {
    throw std::invalid_argument{"unknown benchmark option: " +
                                std::string{option}};
  }

  BenchmarkArguments arguments{
      .scenario_path = argv[2],
      .body_counts = parse_benchmark_body_counts(argv[4]),
      .sort_kind = BenchmarkSortKind::sort,
  };

  if (argc == 7) {
    if (std::string_view{argv[5]} != "--sort") {
      throw std::invalid_argument{"expected --sort after --sizes"};
    }
    arguments.sort_kind = parse_benchmark_sort_kind(argv[6]);
  }

  return arguments;
}

void print_benchmark_header(const Scenario &scenario) {
  std::cout << "OrbitalForge benchmark\n"
            << "======================\n\n"
            << "Scenario template: " << scenario.name << '\n'
            << "Integrators:       all\n"
            << "Steps:             " << scenario.step_count << '\n'
            << "Warmups:           " << default_warmup_count << '\n'
            << "Repetitions:       " << default_repetition_count << "\n\n";

  std::cout << std::right << std::setw(10) << "Bodies" << std::setw(14)
            << "Min (ms)" << std::setw(14) << "Median" << std::setw(14)
            << "Mean" << std::setw(14) << "Stddev" << std::setw(14) << "P95"
            << std::setw(22) << "Milliseconds/step"
            << std::setw(20) << "Checksum" << '\n';

  std::cout << std::string(100, '-') << '\n';
}

[[nodiscard]] int run_benchmark_command(const BenchmarkArguments &arguments) {
  const Scenario scenario = load_scenario(arguments.scenario_path);

  print_benchmark_header(scenario);

  for (const std::size_t body_count : arguments.body_counts) {
    std::vector<IntegratorBenchmarkResult> results = benchmark_all_integrators(
        scenario, body_count, default_warmup_count, default_repetition_count);

    apply_benchmark_sort(results, arguments.sort_kind);

    std::cout << "\nBodies: " << body_count << '\n';

    for (const IntegratorBenchmarkResult &result : results) {
      std::cout << std::left << std::setw(24)
                << integrator_name(result.integrator_kind) << std::right
                << std::fixed << std::setprecision(3) << std::setw(14)
                << result.timing.mean << std::setw(14) << result.timing.p95
                << std::setw(22)
                << result.timing.mean /
                       static_cast<double>(result.step_count)
                << std::setw(20) << result.checksum
                << '\n';
    }
  }

  return success_exit_code;
}

} // namespace

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return usage_error_exit_code;
  }

  const std::string_view command{argv[1]};

  try {
    if (command == "simulate") {
      const SimulateArguments arguments = parse_simulate_arguments(argc, argv);

      return run_simulate_command(arguments);
    }

    if (command == "compare") {
      if (argc != 3) {
        throw std::invalid_argument{
            "compare expects exactly one scenario path"};
      }

      return run_compare_command(argv[2]);
    }

    if (command == "benchmark") {
      const BenchmarkArguments arguments =
          parse_benchmark_arguments(argc, argv);

      return run_benchmark_command(arguments);
    }
    std::cerr << "Unknown command: " << command << "\n\n";

    print_usage(argv[0]);

    return usage_error_exit_code;
  } catch (const std::invalid_argument &error) {
    std::cerr << "Invalid command: " << error.what() << "\n\n";

    print_usage(argv[0]);

    return usage_error_exit_code;
  } catch (const std::exception &error) {
    std::cerr << "OrbitalForge failed: " << error.what() << '\n';

    return simulation_error_exit_code;
  }
}
