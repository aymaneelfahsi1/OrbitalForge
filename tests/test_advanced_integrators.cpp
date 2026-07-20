#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/diagnostics.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/step.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::SystemState;
using orbitalforge::physics::total_momentum;

using StepFunction = void(SystemState &, double, double);

struct NamedIntegrator {
  std::string_view name;
  StepFunction *step;
};

constexpr std::array advanced_integrators{
    NamedIntegrator{
        "Velocity Verlet",
        orbitalforge::simulation::advance_velocity_verlet_step,
    },
    NamedIntegrator{
        "Leapfrog",
        orbitalforge::simulation::advance_leapfrog_step,
    },
    NamedIntegrator{
        "RK4",
        orbitalforge::simulation::advance_runge_kutta_4_step,
    },
};

void require_vec3_approx(const Vec3 &actual, const Vec3 &expected,
                         double margin = 1.0e-12) {

  REQUIRE(actual.x() == Catch::Approx(expected.x()).margin(margin));

  REQUIRE(actual.y() == Catch::Approx(expected.y()).margin(margin));

  REQUIRE(actual.z() == Catch::Approx(expected.z()).margin(margin));
}

void require_system_states_approx(const SystemState &actual,
                                  const SystemState &expected,
                                  double margin = 1.0e-12) {

  REQUIRE(actual.bodies.size() == expected.bodies.size());

  for (std::size_t index = 0; index < actual.bodies.size(); ++index) {

    INFO("body index: " << index);

    require_vec3_approx(actual.bodies[index].position,
                        expected.bodies[index].position, margin);

    require_vec3_approx(actual.bodies[index].velocity,
                        expected.bodies[index].velocity, margin);
  }
}

SystemState make_symmetric_two_body_system() {
  return SystemState{
      .bodies{
          Body{
              "Left",
              1.0,
              Vec3{-1.0, 0.0, 0.0},
              Vec3{},
          },
          Body{
              "Right",
              1.0,
              Vec3{1.0, 0.0, 0.0},
              Vec3{},
          },
      },
  };
}

SystemState make_momentum_test_system() {
  return SystemState{
      .bodies{
          Body{
              "First",
              2.0,
              Vec3{-2.0, 0.0, 0.0},
              Vec3{0.2, 0.7, 0.0},
          },
          Body{
              "Second",
              3.0,
              Vec3{1.0, 1.0, 0.0},
              Vec3{-0.4, 0.1, 0.0},
          },
          Body{
              "Third",
              5.0,
              Vec3{2.0, -1.0, 0.0},
              Vec3{0.3, -0.5, 0.0},
          },
      },
  };
}

} // namespace

TEST_CASE("advanced integrators reject zero time step") {

  constexpr double gravitational_constant = 1.0;

  for (const NamedIntegrator &integrator : advanced_integrators) {

    INFO("integrator: " << integrator.name);

    SystemState system = make_symmetric_two_body_system();

    REQUIRE_THROWS_AS(integrator.step(system, gravitational_constant, 0.0),
                      std::invalid_argument);
  }
}

TEST_CASE("advanced integrators reject negative time step") {

  constexpr double gravitational_constant = 1.0;

  for (const NamedIntegrator &integrator : advanced_integrators) {

    INFO("integrator: " << integrator.name);

    SystemState system = make_symmetric_two_body_system();

    REQUIRE_THROWS_AS(integrator.step(system, gravitational_constant, -0.25),
                      std::invalid_argument);
  }
}

TEST_CASE("advanced integrators reject non-finite time step") {

  constexpr double gravitational_constant = 1.0;

  const double not_a_number = std::numeric_limits<double>::quiet_NaN();

  const double infinity = std::numeric_limits<double>::infinity();

  for (const NamedIntegrator &integrator : advanced_integrators) {

    INFO("integrator: " << integrator.name);

    SystemState nan_system = make_symmetric_two_body_system();

    REQUIRE_THROWS_AS(
        integrator.step(nan_system, gravitational_constant, not_a_number),
        std::invalid_argument);

    SystemState infinity_system = make_symmetric_two_body_system();

    REQUIRE_THROWS_AS(
        integrator.step(infinity_system, gravitational_constant, infinity),
        std::invalid_argument);
  }
}

TEST_CASE("advanced integrators accept an empty system") {

  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.1;

  for (const NamedIntegrator &integrator : advanced_integrators) {

    INFO("integrator: " << integrator.name);

    SystemState system;

    REQUIRE_NOTHROW(integrator.step(system, gravitational_constant, time_step));

    REQUIRE(system.bodies.empty());
  }
}

TEST_CASE("advanced integrators move an isolated body linearly") {

  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.25;

  const Vec3 initial_position{
      10.0,
      -4.0,
      2.0,
  };

  const Vec3 velocity{
      2.0,
      3.0,
      -1.0,
  };

  const Vec3 expected_position = initial_position + velocity * time_step;

  for (const NamedIntegrator &integrator : advanced_integrators) {

    INFO("integrator: " << integrator.name);

    SystemState system{
        .bodies{
            Body{
                "Alone",
                5.0,
                initial_position,
                velocity,
            },
        },
    };

    integrator.step(system, gravitational_constant, time_step);

    require_vec3_approx(system.bodies[0].position, expected_position);

    require_vec3_approx(system.bodies[0].velocity, velocity);
  }
}

TEST_CASE("velocity Verlet performs the expected two-body step") {

  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.5;

  SystemState system = make_symmetric_two_body_system();

  /*
   * Initial body separation:
   *
   *     r = 2
   *
   * Initial acceleration magnitude:
   *
   *     a0 = Gm / r^2
   *        = 1 / 4
   *        = 0.25
   *
   * Velocity-Verlet position:
   *
   *     x1 = x0 + v0 * dt
   *             + 0.5 * a0 * dt^2
   *
   * The initial velocities are zero:
   *
   *     displacement
   *       = 0.5 * 0.25 * 0.5^2
   *       = 0.03125
   */

  orbitalforge::simulation::advance_velocity_verlet_step(
      system, gravitational_constant, time_step);

  constexpr double expected_left_position = -0.96875;

  constexpr double expected_right_position = 0.96875;

  require_vec3_approx(system.bodies[0].position,
                      Vec3{expected_left_position, 0.0, 0.0});

  require_vec3_approx(system.bodies[1].position,
                      Vec3{expected_right_position, 0.0, 0.0});

  const double new_separation =
      expected_right_position - expected_left_position;

  const double final_acceleration =
      gravitational_constant / (new_separation * new_separation);

  const double expected_speed = 0.5 * (0.25 + final_acceleration) * time_step;

  require_vec3_approx(system.bodies[0].velocity,
                      Vec3{expected_speed, 0.0, 0.0});

  require_vec3_approx(system.bodies[1].velocity,
                      Vec3{-expected_speed, 0.0, 0.0});
}

TEST_CASE("leapfrog matches velocity Verlet") {

  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.125;

  SystemState verlet_system{
      .bodies{
          Body{
              "First",
              2.0,
              Vec3{-1.5, 0.2, 0.0},
              Vec3{0.1, 0.4, 0.0},
          },
          Body{
              "Second",
              3.0,
              Vec3{1.0, -0.3, 0.0},
              Vec3{-0.2, -0.1, 0.0},
          },
          Body{
              "Third",
              0.5,
              Vec3{0.2, 2.0, 0.0},
              Vec3{0.3, -0.2, 0.0},
          },
      },
  };

  SystemState leapfrog_system = verlet_system;

  orbitalforge::simulation::advance_velocity_verlet_step(
      verlet_system, gravitational_constant, time_step);

  orbitalforge::simulation::advance_leapfrog_step(
      leapfrog_system, gravitational_constant, time_step);

  require_system_states_approx(leapfrog_system, verlet_system);
}

TEST_CASE("RK4 preserves mirror symmetry") {

  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.2;

  SystemState system{
      .bodies{
          Body{
              "Left",
              1.0,
              Vec3{-1.0, 0.0, 0.0},
              Vec3{0.0, -0.5, 0.0},
          },
          Body{
              "Right",
              1.0,
              Vec3{1.0, 0.0, 0.0},
              Vec3{0.0, 0.5, 0.0},
          },
      },
  };

  orbitalforge::simulation::advance_runge_kutta_4_step(
      system, gravitational_constant, time_step);

  const Body &left = system.bodies[0];

  const Body &right = system.bodies[1];

  REQUIRE(left.position.x() ==
          Catch::Approx(-right.position.x()).margin(1.0e-12));

  REQUIRE(left.position.y() ==
          Catch::Approx(-right.position.y()).margin(1.0e-12));

  REQUIRE(left.position.z() ==
          Catch::Approx(-right.position.z()).margin(1.0e-12));

  REQUIRE(left.velocity.x() ==
          Catch::Approx(-right.velocity.x()).margin(1.0e-12));

  REQUIRE(left.velocity.y() ==
          Catch::Approx(-right.velocity.y()).margin(1.0e-12));

  REQUIRE(left.velocity.z() ==
          Catch::Approx(-right.velocity.z()).margin(1.0e-12));
}

TEST_CASE("advanced integrators preserve total momentum") {

  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.05;

  for (const NamedIntegrator &integrator : advanced_integrators) {

    INFO("integrator: " << integrator.name);

    SystemState system = make_momentum_test_system();

    const Vec3 initial_momentum = total_momentum(system);

    integrator.step(system, gravitational_constant, time_step);

    const Vec3 final_momentum = total_momentum(system);

    require_vec3_approx(final_momentum, initial_momentum, 1.0e-11);
  }
}

TEST_CASE("advanced integrators preserve body identity and mass") {

  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.1;

  for (const NamedIntegrator &integrator : advanced_integrators) {

    INFO("integrator: " << integrator.name);

    SystemState system = make_momentum_test_system();

    const std::string first_name = system.bodies[0].name;

    const std::string second_name = system.bodies[1].name;

    const std::string third_name = system.bodies[2].name;

    const double first_mass = system.bodies[0].mass;

    const double second_mass = system.bodies[1].mass;

    const double third_mass = system.bodies[2].mass;

    integrator.step(system, gravitational_constant, time_step);

    REQUIRE(system.bodies[0].name == first_name);

    REQUIRE(system.bodies[1].name == second_name);

    REQUIRE(system.bodies[2].name == third_name);

    REQUIRE(system.bodies[0].mass == first_mass);

    REQUIRE(system.bodies[1].mass == second_mass);

    REQUIRE(system.bodies[2].mass == third_mass);
  }
}