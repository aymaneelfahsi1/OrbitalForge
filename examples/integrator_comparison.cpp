#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/diagnostics.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/step.hpp"

namespace {

using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::center_of_mass;
using orbitalforge::physics::SystemState;
using orbitalforge::physics::total_energy;
using orbitalforge::physics::total_momentum;
using orbitalforge::simulation::advance_explicit_euler_step;
using orbitalforge::simulation::advance_leapfrog_step;
using orbitalforge::simulation::advance_runge_kutta_4_step;
using orbitalforge::simulation::advance_semi_implicit_euler_step;
using orbitalforge::simulation::advance_velocity_verlet_step;

using StepFunction = void (*)(SystemState &, double, double, double);

struct Integrator {
  std::string_view name;
  StepFunction step;
};

struct SimulationResult {
  std::string_view integrator_name;
  double initial_energy;
  double final_energy;
  double absolute_energy_drift;
  double relative_energy_drift;
  double momentum_drift;
  double center_of_mass_drift;
  double final_separation;
  double elapsed_milliseconds;
};

constexpr std::array integrators{
    Integrator{
        "Explicit Euler",
        advance_explicit_euler_step,
    },
    Integrator{
        "Semi-implicit Euler",
        advance_semi_implicit_euler_step,
    },
    Integrator{
        "Velocity Verlet",
        advance_velocity_verlet_step,
    },
    Integrator{
        "Leapfrog",
        advance_leapfrog_step,
    },
    Integrator{
        "RK4",
        advance_runge_kutta_4_step,
    },
};

[[nodiscard]] SystemState make_equal_mass_circular_orbit() {
  const double orbital_speed = std::sqrt(0.5);

  return SystemState{
      .bodies{
          Body{
              "Primary",
              1.0,
              Vec3{-0.5, 0.0, 0.0},
              Vec3{0.0, -orbital_speed, 0.0},
          },
          Body{
              "Secondary",
              1.0,
              Vec3{0.5, 0.0, 0.0},
              Vec3{0.0, orbital_speed, 0.0},
          },
      },
  };
}

[[nodiscard]] double relative_drift(double initial_value, double final_value) {
  const double absolute_drift = std::abs(final_value - initial_value);

  if (initial_value == 0.0) {
    return absolute_drift;
  }

  return absolute_drift / std::abs(initial_value);
}

[[nodiscard]] double body_separation(const SystemState &system) {
  if (system.bodies.size() < 2) {
    return 0.0;
  }

  const Vec3 displacement =
      system.bodies[1].position - system.bodies[0].position;

  return displacement.norm();
}

[[nodiscard]] SimulationResult
run_simulation(const Integrator &integrator, const SystemState &initial_system,
               double gravitational_constant, double softening,
               double time_step, std::size_t step_count) {
  SystemState system = initial_system;

  const double initial_energy =
      total_energy(system, gravitational_constant, softening);

  const Vec3 initial_momentum = total_momentum(system);

  const Vec3 initial_center_of_mass = center_of_mass(system);

  const auto start_time = std::chrono::steady_clock::now();

  for (std::size_t step = 0; step < step_count; ++step) {
    integrator.step(system, gravitational_constant, softening, time_step);
  }

  const auto end_time = std::chrono::steady_clock::now();

  const double final_energy =
      total_energy(system, gravitational_constant, softening);

  const Vec3 final_momentum = total_momentum(system);

  const Vec3 final_center_of_mass = center_of_mass(system);

  const double absolute_energy_drift = std::abs(final_energy - initial_energy);

  const double momentum_drift = (final_momentum - initial_momentum).norm();

  const double center_of_mass_drift =
      (final_center_of_mass - initial_center_of_mass).norm();

  const std::chrono::duration<double, std::milli> elapsed_time =
      end_time - start_time;

  return SimulationResult{
      .integrator_name = integrator.name,
      .initial_energy = initial_energy,
      .final_energy = final_energy,
      .absolute_energy_drift = absolute_energy_drift,
      .relative_energy_drift = relative_drift(initial_energy, final_energy),
      .momentum_drift = momentum_drift,
      .center_of_mass_drift = center_of_mass_drift,
      .final_separation = body_separation(system),
      .elapsed_milliseconds = elapsed_time.count(),
  };
}

void print_header(double gravitational_constant, double softening,
                  double time_step, std::size_t step_count) {
  const double total_time = time_step * static_cast<double>(step_count);

  std::cout << "OrbitalForge integrator comparison\n"
            << "==================================\n\n"
            << "Scenario:               "
            << "equal-mass circular binary orbit\n"
            << "Gravitational constant: " << gravitational_constant << '\n'
            << "Softening:              " << softening << '\n'
            << "Time step:              " << time_step << '\n'
            << "Step count:             " << step_count << '\n'
            << "Total simulated time:   " << total_time << "\n\n";
}

void print_table_header() {
  std::cout << std::left << std::setw(22) << "Integrator"

            << std::right << std::setw(18) << "Abs E drift"

            << std::setw(18) << "Rel E drift"

            << std::setw(18) << "Momentum drift"

            << std::setw(18) << "COM drift"

            << std::setw(18) << "Final distance"

            << std::setw(14) << "Time (ms)"

            << '\n';

  std::cout << std::string(126, '-') << '\n';
}

void print_result(const SimulationResult &result) {
  std::cout << std::left << std::setw(22) << result.integrator_name

            << std::right << std::scientific << std::setprecision(6)

            << std::setw(18) << result.absolute_energy_drift

            << std::setw(18) << result.relative_energy_drift

            << std::setw(18) << result.momentum_drift

            << std::setw(18) << result.center_of_mass_drift

            << std::setw(18) << result.final_separation

            << std::fixed << std::setprecision(3) << std::setw(14)
            << result.elapsed_milliseconds

            << '\n';
}

void print_energy_details(const SimulationResult &result) {
  std::cout << '\n'
            << result.integrator_name << '\n'

            << "  Initial energy: " << std::scientific << std::setprecision(12)
            << result.initial_energy << '\n'

            << "  Final energy:   " << result.final_energy << '\n'

            << "  Energy change:  "
            << result.final_energy - result.initial_energy << '\n';
}

} // namespace

int main() {
  constexpr double gravitational_constant = 1.0;

  constexpr double softening = 0.0;

  constexpr double time_step = 0.001;

  constexpr std::size_t step_count = 100'000;

  const SystemState initial_system = make_equal_mass_circular_orbit();

  print_header(gravitational_constant, softening, time_step, step_count);

  print_table_header();

  std::array<SimulationResult, integrators.size()> results{};

  for (std::size_t index = 0; index < integrators.size(); ++index) {
    results[index] = run_simulation(integrators[index], initial_system,
                                    gravitational_constant, softening,
                                    time_step, step_count);

    print_result(results[index]);
  }

  std::cout << "\nEnergy details:\n";

  for (const SimulationResult &result : results) {
    print_energy_details(result);
  }

  return 0;
}