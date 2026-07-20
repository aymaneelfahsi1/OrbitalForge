#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/diagnostics.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/step.hpp"

namespace {

using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::SystemState;

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

    system.bodies.push_back(Body{
        "Star",
        star_mass,
        position,
        velocity,
    });
  }

  const Vec3 initial_center_of_mass =
      orbitalforge::physics::center_of_mass(system);

  for (Body &body : system.bodies) {
    body.position -= initial_center_of_mass;
  }

  const Vec3 initial_momentum = orbitalforge::physics::total_momentum(system);

  const Vec3 center_of_mass_velocity =
      initial_momentum / (star_mass * static_cast<double>(body_count));

  for (Body &body : system.bodies) {
    body.velocity -= center_of_mass_velocity;
  }

  return system;
}

[[nodiscard]] FILE *open_live_plot() {

  FILE *plot = popen("gnuplot", "w");

  if (plot == nullptr) {
    std::cerr << "Warning: Gnuplot could not be started.\n"
              << "The simulation will continue without visualization.\n";

    return nullptr;
  }

  std::fprintf(plot, "set terminal qt size 1000,800\n"
                     "set title 'OrbitalForge N-body simulation'\n"
                     "set xlabel 'x (AU)'\n"
                     "set ylabel 'y (AU)'\n"
                     "set zlabel 'z (AU)'\n"
                     "set grid\n"
                     "set view 60,30\n"
                     "set xrange [-1100:1100]\n"
                     "set yrange [-1100:1100]\n"
                     "set zrange [-100:100]\n"
                     "set ticslevel 0\n"
                     "set key off\n");

  std::fflush(plot);

  return plot;
}

void update_live_plot(FILE *plot, const SystemState &system,
                      double simulation_time, double elapsed_real_time) {

  if (plot == nullptr) {
    return;
  }

  std::fprintf(plot,
               "set title "
               "'OrbitalForge | simulated: %.2f days | real: %.2f s'\n",
               simulation_time / seconds_per_day, elapsed_real_time);

  std::fprintf(plot, "splot '-' using 1:2:3 "
                     "with points pointtype 7 pointsize 0.7\n");

  for (const Body &body : system.bodies) {
    std::fprintf(plot, "%.12e %.12e %.12e\n",
                 body.position.x() / astronomical_unit,
                 body.position.y() / astronomical_unit,
                 body.position.z() / astronomical_unit);
  }

  std::fprintf(plot, "e\n");

  std::fflush(plot);
}

void close_live_plot(FILE *plot) {

  if (plot == nullptr) {
    return;
  }

  std::fprintf(plot, "exit\n");
  std::fflush(plot);
  pclose(plot);
}

void print_progress(const SystemState &system, std::size_t step,
                    double simulation_time, double elapsed_real_time,
                    double initial_energy, const Vec3 &initial_momentum,
                    const Vec3 &initial_center_of_mass) {

  const double current_energy =
      orbitalforge::physics::total_energy(system, gravitational_constant);

  const Vec3 current_momentum = orbitalforge::physics::total_momentum(system);

  const Vec3 current_center_of_mass =
      orbitalforge::physics::center_of_mass(system);

  const double absolute_energy_drift =
      std::abs(current_energy - initial_energy);

  const double relative_energy_drift =
      absolute_energy_drift / std::abs(initial_energy);

  const double momentum_drift = (current_momentum - initial_momentum).norm();

  const double center_of_mass_drift =
      (current_center_of_mass - initial_center_of_mass).norm();

  std::cout << std::right

            << std::setw(10) << step

            << std::setw(16) << std::fixed << std::setprecision(2)
            << simulation_time / seconds_per_day

            << std::setw(16) << elapsed_real_time

            << std::setw(22) << std::scientific << std::setprecision(8)
            << current_energy

            << std::setw(20) << relative_energy_drift

            << std::setw(20) << momentum_drift

            << std::setw(20) << center_of_mass_drift

            << '\n';
}

} // namespace

int main() {

  constexpr std::size_t body_count = 20;

  constexpr std::size_t step_count = 2'000'000;

  constexpr std::size_t output_interval = 200;

  constexpr std::size_t plot_interval = 1;

  constexpr double time_step = 0.1 * seconds_per_day;

  SystemState system = make_rotating_star_cluster(body_count);

  const double initial_energy =
      orbitalforge::physics::total_energy(system, gravitational_constant);

  const Vec3 initial_momentum = orbitalforge::physics::total_momentum(system);

  const Vec3 initial_center_of_mass =
      orbitalforge::physics::center_of_mass(system);

  const double simulated_duration = time_step * static_cast<double>(step_count);

  std::cout << "OrbitalForge N-body Simulator\n"
            << "==============================\n\n"

            << "Scenario:             "
            << "self-gravitating rotating star cluster\n"

            << "Integrator:           "
            << "Velocity Verlet\n"

            << "Unit system:          "
            << "SI\n"

            << "Gravitational G:      " << std::scientific
            << gravitational_constant << " m^3 kg^-1 s^-2\n"

            << "Bodies:               " << body_count << '\n'

            << "Mass per body:        " << solar_mass << " kg\n"

            << "Time step:            " << std::fixed << std::setprecision(2)
            << time_step / seconds_per_day << " days\n"

            << "Simulation duration:  " << simulated_duration / seconds_per_day
            << " days\n"

            << "Steps:                " << step_count << "\n\n";

  std::cout << std::right

            << std::setw(10) << "Step"

            << std::setw(16) << "Time (days)"

            << std::setw(16) << "Real time (s)"

            << std::setw(22) << "Energy (J)"

            << std::setw(20) << "Rel E drift"

            << std::setw(20) << "Momentum drift"

            << std::setw(20) << "COM drift (m)"

            << '\n';

  std::cout << std::string(124, '-') << '\n';

  FILE *live_plot = open_live_plot();

  const auto start_time = std::chrono::steady_clock::now();

  print_progress(system, 0, 0.0, 0.0, initial_energy, initial_momentum,
                 initial_center_of_mass);

  update_live_plot(live_plot, system, 0.0, 0.0);

  for (std::size_t step = 1; step <= step_count; ++step) {

    orbitalforge::simulation::advance_velocity_verlet_step(
        system, gravitational_constant, time_step);

    const double simulation_time = static_cast<double>(step) * time_step;

    const auto current_time = std::chrono::steady_clock::now();

    const std::chrono::duration<double> elapsed_real_time =
        current_time - start_time;

    if (step % plot_interval == 0 || step == step_count) {

      update_live_plot(live_plot, system, simulation_time,
                       elapsed_real_time.count());
    }

    if (step % output_interval == 0 || step == step_count) {

      print_progress(system, step, simulation_time, elapsed_real_time.count(),
                     initial_energy, initial_momentum, initial_center_of_mass);
    }
  }

  const auto end_time = std::chrono::steady_clock::now();

  const std::chrono::duration<double, std::milli> elapsed_time =
      end_time - start_time;

  const double final_energy =
      orbitalforge::physics::total_energy(system, gravitational_constant);

  const Vec3 final_momentum = orbitalforge::physics::total_momentum(system);

  const Vec3 final_center_of_mass =
      orbitalforge::physics::center_of_mass(system);

  const double absolute_energy_drift = std::abs(final_energy - initial_energy);

  const double relative_energy_drift =
      absolute_energy_drift / std::abs(initial_energy);

  const double momentum_drift = (final_momentum - initial_momentum).norm();

  const double center_of_mass_drift =
      (final_center_of_mass - initial_center_of_mass).norm();

  std::cout << "\nFinal diagnostics\n"
            << "=================\n"

            << std::scientific << std::setprecision(12)

            << "Initial energy:         " << initial_energy << " J\n"

            << "Final energy:           " << final_energy << " J\n"

            << "Absolute energy drift:  " << absolute_energy_drift << " J\n"

            << "Relative energy drift:  " << relative_energy_drift << '\n'

            << "Momentum drift:         " << momentum_drift << " kg m/s\n"

            << "Center-of-mass drift:   " << center_of_mass_drift << " m\n"

            << std::fixed << std::setprecision(3)

            << "Execution time:         " << elapsed_time.count() << " ms\n";

  close_live_plot(live_plot);

  return 0;
}