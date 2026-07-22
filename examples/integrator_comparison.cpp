
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string_view>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/diagnostics.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/step.hpp"

namespace {

using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::SystemState;
using orbitalforge::physics::total_momentum;

using StepFunction = void (*)(SystemState &, double, double);

using orbitalforge::physics::center_of_mass;
using orbitalforge::physics::total_energy;
using orbitalforge::physics::total_momentum;
using orbitalforge::simulation::advance_explicit_euler_step;
using orbitalforge::simulation::advance_leapfrog_step;
using orbitalforge::simulation::advance_runge_kutta_4_step;
using orbitalforge::simulation::advance_semi_implicit_euler_step;
using orbitalforge::simulation::advance_velocity_verlet_step;

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
    Integrator{"Explicit Euler", advance_explicit_euler_step},
    Integrator{"Semi-implicit  Euler ", advance_semi_implicit_euler_step},
    Integrator{"Velocity Verlet", advance_velocity_verlet_step},
    Integrator{"Leapfrog", advance_leapfrog_step},
    Integrator{"RK4", advance_runge_kutta_4_step}};

[[nodiscard]] SystemState make_equal_mass_circular_orbit() {
  /*
   * We use normalized units:
   *
   *     G = 1
   *     m1 = 1
   *     m2 = 1
   *
   * The bodies are separated by distance:
   *
   *     r = 1
   *
   * Their center of mass is at the origin:
   *
   *     x1 = -0.5
   *     x2 =  0.5
   *
   * Each body moves on a circle of radius:
   *
   *     R = 0.5
   *
   * Gravitational acceleration magnitude:
   *
   *     a = Gm / r^2 = 1
   *
   * Circular motion requires:
   *
   *     v^2 / R = a
   *
   * Therefore:
   *
   *     v = sqrt(aR)
   *       = sqrt(0.5)
   */

  const double orbital_speed = std::sqrt(.5);

  return SystemState{.bodies{
      Body{"Primary", 1.0, Vec3{-.5, .0, .0}, Vec3{.0, -orbital_speed, .0}},
      Body{"Secondary", 1.0, Vec3{.5, .0, .0}, Vec3{.0, orbital_speed, .0}}}};
}

[[nodiscard]] double relative_drift(double initial_value, double final_value) {
  const double absolute_drift = std::abs(final_value - initial_value);

  if (initial_value == .0) {
    return absolute_drift;
  }

  return absolute_drift / std::abs(initial_value);
}

[[nodiscard]] double body_seperation(const SystemState &system) {
  if (system.bodies.size() < 2) {
    return .0;
  }

  const Vec3 displacement =
      system.bodies[1].position - system.bodies[0].position;
  return displacement.norm();
}

[[nodiscard]] SimulationResult run_simulation(const Integrator &integrator,
                                              const SystemState &initial_system,
                                              double gravitational_constant,
                                              double time_step,
                                              std::size_t step_count) {
  SystemState system = initial_system;

  const double initial_energy = total_energy(system, gravitational_constant);

  const Vec3 initial_momentum = total_momentum(system);

  const Vec3 initial_center_of_mass = center_of_mass(system);

  const auto start_time = std::chrono::steady_clock::now();

  for (std::size_t step = 0; step < step_count; ++step) {
    integrator.step(system, gravitational_constant, time_step);
  }

  const auto end_time = std::chrono::steady_clock::now();
  const double final_energy = total_energy(system, gravitational_constant);

  const Vec3 final_momentum = total_momentum(system);

  const Vec3 final_center_of_mass = center_of_mass(system);

  const double absolute_energy_drift = std::abs(final_energy - initial_energy);
  const double momentum_drift = (final_momentum - initial_momentum).norm();
  const double center_of_mass_drift =
      (final_center_of_mass - initial_center_of_mass).norm();

  const std::chrono::duration<double, std::milli> elapsed_time =
      end_time - start_time;

  return SimulationResult{.integrator_name = integrator.name,
                          .initial_energy = initial_energy,
                          .final_energy = final_energy,
                          .absolute_energy_drift = absolute_energy_drift,
                          .relative_energy_drift =
                              relative_drift(initial_energy, final_energy),
                          .momentum_drift = momentum_drift,
                          .center_of_mass_drift = center_of_mass_drift,
                          .final_separation = body_seperation(system),
                          .elapsed_milliseconds = elapsed_time.count()};
}

void print_header(double gravitational_constant, double time_step,
                  std::size_t step_count) {

  const double total_time = time_step * static_cast<double>(step_count);

  std::cout << "OrbitalForge integrator comparison\n"
            << "==================================\n\n";

  std::cout << "Scenario:               "
            << "equal-mass circular binary orbit\n";

  std::cout << "Gravitational constant: " << gravitational_constant << '\n';

  std::cout << "Time step:              " << time_step << '\n';

  std::cout << "Step count:             " << step_count << '\n';

  std::cout << "Total simulated time:   " << total_time << "\n\n";
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

  std::cout << "\n" << result.integrator_name << '\n';

  std::cout << "  Initial energy: " << std::scientific << std::setprecision(12)
            << result.initial_energy << '\n';

  std::cout << "  Final energy:   " << result.final_energy << '\n';

  std::cout << "  Energy change:  "
            << result.final_energy - result.initial_energy << '\n';
}

} // namespace

int main() {
  constexpr double gravitational_constant = 1.0;

  constexpr double time_step = 0.01;

  constexpr std::size_t step_count = 100'000;

  const SystemState initial_system = make_equal_mass_circular_orbit();

  print_header(gravitational_constant, time_step, step_count);

  print_table_header();

  std::array<SimulationResult, integrators.size()> results{};

  for (std::size_t index = 0; index < integrators.size(); ++index) {
    results[index] =
        run_simulation(integrators[index], initial_system,
                       gravitational_constant, time_step, step_count);

    print_result(results[index]);
  }

  std::cout << "\n Energy details: \n" << "\n";

  for (const SimulationResult &result : results) {
    print_energy_details(result);
  }

  return 0;
}
