#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/diagnostics.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"
#include "orbitalforge/simulation/simulation_runner.hpp"

namespace {

using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::center_of_mass;
using orbitalforge::physics::SystemState;
using orbitalforge::physics::total_momentum;
using orbitalforge::simulation::DiagnosticSample;
using orbitalforge::simulation::integrator_name;
using orbitalforge::simulation::IntegratorKind;
using orbitalforge::simulation::SimulationConfig;
using orbitalforge::simulation::SimulationResult;
using orbitalforge::simulation::SimulationRunner;

constexpr double gravitational_constant = 6.67430e-11;
constexpr double solar_mass = 1.98847e30;
constexpr double astronomical_unit = 1.495978707e11;
constexpr double seconds_per_day = 86'400.0;

[[nodiscard]] SystemState make_rotating_star_cluster(std::size_t body_count) {
  SystemState system{};

  system.bodies.reserve(body_count);

  if (body_count == 0) {
    return system;
  }

  constexpr double minimum_radius = 10.0 * astronomical_unit;
  constexpr double maximum_radius = 1'000.0 * astronomical_unit;
  constexpr double star_mass = solar_mass;
  constexpr double golden_angle = 2.39996322972865332;

  for (std::size_t index = 0; index < body_count; ++index) {
    const double fraction =
        (static_cast<double>(index) + 1.0) / static_cast<double>(body_count);

    const double radius =
        minimum_radius +
        std::sqrt(fraction) * (maximum_radius - minimum_radius);

    const double angle = golden_angle * static_cast<double>(index);

    const double enclosed_mass = star_mass * static_cast<double>(index + 1);

    const double orbital_speed =
        std::sqrt(gravitational_constant * enclosed_mass / radius);

    const double vertical_fraction =
        static_cast<double>(static_cast<int>(index % 11) - 5) / 5.0;

    const double height = vertical_fraction * 2.0 * astronomical_unit;

    const Vec3 position{
        radius * std::cos(angle),
        radius * std::sin(angle),
        height,
    };

    const Vec3 velocity{
        -orbital_speed * std::sin(angle),
        orbital_speed * std::cos(angle),
        0.0,
    };

    system.bodies.emplace_back("Star-" + std::to_string(index + 1), star_mass,
                               position, velocity);
  }

  const Vec3 initial_center_of_mass = center_of_mass(system);

  for (Body &body : system.bodies) {
    body.position -= initial_center_of_mass;
  }

  const Vec3 initial_momentum = total_momentum(system);

  const Vec3 center_of_mass_velocity =
      initial_momentum / (star_mass * static_cast<double>(body_count));

  for (Body &body : system.bodies) {
    body.velocity -= center_of_mass_velocity;
  }

  return system;
}

void print_simulation_header(const SimulationConfig &config,
                             std::size_t body_count) {
  const double simulated_duration =
      config.time_step * static_cast<double>(config.step_count);

  std::cout << "OrbitalForge N-body Simulator\n"
            << "==============================\n\n"
            << "Scenario:             "
            << "self-gravitating rotating star cluster\n"
            << "Integrator:           " << integrator_name(config.integrator)
            << '\n'
            << "Unit system:          SI\n"
            << "Gravitational G:      " << std::scientific
            << gravitational_constant << " m^3 kg^-1 s^-2\n"
            << "Bodies:               " << body_count << '\n'
            << "Mass per body:        " << solar_mass << " kg\n"
            << "Time step:            " << std::fixed << std::setprecision(2)
            << config.time_step / seconds_per_day << " days\n"
            << "Simulation duration:  " << simulated_duration / seconds_per_day
            << " days\n"
            << "Steps:                " << config.step_count << "\n\n";
}

void print_diagnostics_header() {
  std::cout << std::right << std::setw(10) << "Step" << std::setw(16)
            << "Time (days)" << std::setw(22) << "Energy (J)" << std::setw(20)
            << "Rel E drift" << std::setw(20) << "Momentum drift"
            << std::setw(20) << "COM drift (m)" << '\n';

  std::cout << std::string(108, '-') << '\n';
}

void print_diagnostic_sample(const DiagnosticSample &sample) {
  std::cout << std::right << std::setw(10) << sample.step

            << std::setw(16) << std::fixed << std::setprecision(2)
            << sample.simulation_time / seconds_per_day

            << std::setw(22) << std::scientific << std::setprecision(8)
            << sample.total_energy

            << std::setw(20) << sample.relative_energy_drift

            << std::setw(20) << sample.momentum_drift

            << std::setw(20) << sample.center_of_mass_drift

            << '\n';
}

void print_final_diagnostics(const SimulationResult &result,
                             double elapsed_milliseconds) {
  const DiagnosticSample &initial_sample = result.diagnostics.front();

  const DiagnosticSample &final_sample = result.diagnostics.back();

  const double absolute_energy_drift =
      std::abs(final_sample.total_energy - initial_sample.total_energy);

  std::cout << "\nFinal diagnostics\n"
            << "=================\n"
            << std::scientific << std::setprecision(12)
            << "Initial energy:         " << initial_sample.total_energy
            << " J\n"
            << "Final energy:           " << final_sample.total_energy << " J\n"
            << "Absolute energy drift:  " << absolute_energy_drift << " J\n"
            << "Relative energy drift:  " << final_sample.relative_energy_drift
            << '\n'
            << "Momentum drift:         " << final_sample.momentum_drift
            << " kg m/s\n"
            << "Center-of-mass drift:   " << final_sample.center_of_mass_drift
            << " m\n"
            << std::fixed << std::setprecision(3)
            << "Execution time:         " << elapsed_milliseconds << " ms\n";
}

} // namespace

int main() {
  constexpr std::size_t body_count = 20;
  constexpr std::size_t step_count = 2'000;
  constexpr std::size_t output_interval = 200;
  constexpr double time_step = 10.0 * seconds_per_day;

  const SystemState initial_system = make_rotating_star_cluster(body_count);

  const SimulationConfig config{
      .gravitational_constant = gravitational_constant,
      .time_step = time_step,
      .step_count = step_count,
      .output_interval = output_interval,
      .integrator = IntegratorKind::velocity_verlet,
  };

  print_simulation_header(config, body_count);
  print_diagnostics_header();

  const SimulationRunner runner;

  const auto start_time = std::chrono::steady_clock::now();

  const SimulationResult result = runner.run(initial_system, config);

  const auto end_time = std::chrono::steady_clock::now();

  const std::chrono::duration<double, std::milli> elapsed_time =
      end_time - start_time;

  for (const DiagnosticSample &sample : result.diagnostics) {
    print_diagnostic_sample(sample);
  }

  print_final_diagnostics(result, elapsed_time.count());

  return 0;
}