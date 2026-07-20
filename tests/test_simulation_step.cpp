#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/step.hpp"

namespace {

using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::SystemState;

void require_vec3_approx(const Vec3 &actual, const Vec3 &expected,
                         double margin = 1.0e-12) {
  REQUIRE(actual.x() == Catch::Approx(expected.x()).margin(margin));
  REQUIRE(actual.y() == Catch::Approx(expected.y()).margin(margin));
  REQUIRE(actual.z() == Catch::Approx(expected.z()).margin(margin));
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

} // namespace

TEST_CASE("explicit Euler advances a symmetric two-body system") {
  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.5;

  SystemState system = make_symmetric_two_body_system();

  orbitalforge::simulation::advance_explicit_euler_step(
      system, gravitational_constant, time_step);

  /*
   * Initial distance between the bodies is 2.
   *
   * Acceleration magnitude:
   *
   *     a = Gm / r^2
   *       = 1 * 1 / 2^2
   *       = 0.25
   *
   * Explicit Euler uses the old velocity for position:
   *
   *     x_next = x + v * dt
   *
   * Both initial velocities are zero, so positions remain unchanged.
   *
   *     v_next = v + a * dt
   *            = 0 + 0.25 * 0.5
   *            = 0.125
   */

  require_vec3_approx(system.bodies[0].position, Vec3{-1.0, 0.0, 0.0});

  require_vec3_approx(system.bodies[1].position, Vec3{1.0, 0.0, 0.0});

  require_vec3_approx(system.bodies[0].velocity, Vec3{0.125, 0.0, 0.0});

  require_vec3_approx(system.bodies[1].velocity, Vec3{-0.125, 0.0, 0.0});
}

TEST_CASE("semi-implicit Euler advances a symmetric two-body system") {
  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.5;

  SystemState system = make_symmetric_two_body_system();

  orbitalforge::simulation::advance_semi_implicit_euler_step(
      system, gravitational_constant, time_step);

  /*
   * The acceleration magnitude is 0.25.
   *
   * Semi-implicit Euler first updates velocity:
   *
   *     v_next = 0 + 0.25 * 0.5
   *            = 0.125
   *
   * It then updates position using the new velocity:
   *
   *     x_next = x + v_next * dt
   *            = x + 0.125 * 0.5
   *            = x + 0.0625
   */

  require_vec3_approx(system.bodies[0].velocity, Vec3{0.125, 0.0, 0.0});

  require_vec3_approx(system.bodies[1].velocity, Vec3{-0.125, 0.0, 0.0});

  require_vec3_approx(system.bodies[0].position, Vec3{-0.9375, 0.0, 0.0});

  require_vec3_approx(system.bodies[1].position, Vec3{0.9375, 0.0, 0.0});
}

TEST_CASE("explicit Euler computes all accelerations from the same snapshot") {
  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.25;

  SystemState system = make_symmetric_two_body_system();

  orbitalforge::simulation::advance_explicit_euler_step(
      system, gravitational_constant, time_step);

  /*
   * Both bodies must be updated from accelerations calculated before either
   * body is mutated. Otherwise, updating the first body could influence the
   * acceleration used for the second body.
   */

  REQUIRE(system.bodies[0].velocity.x() ==
          Catch::Approx(-system.bodies[1].velocity.x()));

  REQUIRE(system.bodies[0].velocity.y() ==
          Catch::Approx(-system.bodies[1].velocity.y()));

  REQUIRE(system.bodies[0].velocity.z() ==
          Catch::Approx(-system.bodies[1].velocity.z()));
}

TEST_CASE(
    "semi-implicit Euler computes all accelerations from the same snapshot") {
  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.25;

  SystemState system = make_symmetric_two_body_system();

  orbitalforge::simulation::advance_semi_implicit_euler_step(
      system, gravitational_constant, time_step);

  REQUIRE(system.bodies[0].position.x() ==
          Catch::Approx(-system.bodies[1].position.x()));

  REQUIRE(system.bodies[0].position.y() ==
          Catch::Approx(-system.bodies[1].position.y()));

  REQUIRE(system.bodies[0].position.z() ==
          Catch::Approx(-system.bodies[1].position.z()));

  REQUIRE(system.bodies[0].velocity.x() ==
          Catch::Approx(-system.bodies[1].velocity.x()));

  REQUIRE(system.bodies[0].velocity.y() ==
          Catch::Approx(-system.bodies[1].velocity.y()));

  REQUIRE(system.bodies[0].velocity.z() ==
          Catch::Approx(-system.bodies[1].velocity.z()));
}

TEST_CASE("simulation steps preserve body names and masses") {
  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.1;

  SystemState system = make_symmetric_two_body_system();

  orbitalforge::simulation::advance_semi_implicit_euler_step(
      system, gravitational_constant, time_step);

  REQUIRE(system.bodies[0].name == "Left");
  REQUIRE(system.bodies[0].mass == Catch::Approx(1.0));

  REQUIRE(system.bodies[1].name == "Right");
  REQUIRE(system.bodies[1].mass == Catch::Approx(1.0));
}

TEST_CASE(
    "an isolated body moves with constant velocity using explicit Euler") {
  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.5;

  SystemState system{
      .bodies{
          Body{
              "Solo",
              5.0,
              Vec3{1.0, 2.0, 3.0},
              Vec3{4.0, -2.0, 1.0},
          },
      },
  };

  orbitalforge::simulation::advance_explicit_euler_step(
      system, gravitational_constant, time_step);

  require_vec3_approx(system.bodies[0].position, Vec3{3.0, 1.0, 3.5});

  require_vec3_approx(system.bodies[0].velocity, Vec3{4.0, -2.0, 1.0});
}

TEST_CASE(
    "an isolated body moves with constant velocity using semi-implicit Euler") {
  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.5;

  SystemState system{
      .bodies{
          Body{
              "Solo",
              5.0,
              Vec3{1.0, 2.0, 3.0},
              Vec3{4.0, -2.0, 1.0},
          },
      },
  };

  orbitalforge::simulation::advance_semi_implicit_euler_step(
      system, gravitational_constant, time_step);

  /*
   * With zero acceleration, explicit and semi-implicit Euler produce the
   * same result because the velocity does not change.
   */

  require_vec3_approx(system.bodies[0].position, Vec3{3.0, 1.0, 3.5});

  require_vec3_approx(system.bodies[0].velocity, Vec3{4.0, -2.0, 1.0});
}

TEST_CASE("simulation steps leave an empty system empty") {
  constexpr double gravitational_constant = 1.0;
  constexpr double time_step = 0.1;

  SystemState explicit_system{};

  orbitalforge::simulation::advance_explicit_euler_step(
      explicit_system, gravitational_constant, time_step);

  REQUIRE(explicit_system.bodies.empty());

  SystemState semi_implicit_system{};

  orbitalforge::simulation::advance_semi_implicit_euler_step(
      semi_implicit_system, gravitational_constant, time_step);

  REQUIRE(semi_implicit_system.bodies.empty());
}

TEST_CASE("explicit Euler rejects a zero time step") {
  SystemState system = make_symmetric_two_body_system();

  REQUIRE_THROWS_AS(
      orbitalforge::simulation::advance_explicit_euler_step(system, 1.0, 0.0),
      std::invalid_argument);
}

TEST_CASE("explicit Euler rejects a negative time step") {
  SystemState system = make_symmetric_two_body_system();

  REQUIRE_THROWS_AS(
      orbitalforge::simulation::advance_explicit_euler_step(system, 1.0, -0.1),
      std::invalid_argument);
}

TEST_CASE("semi-implicit Euler rejects a zero time step") {
  SystemState system = make_symmetric_two_body_system();

  REQUIRE_THROWS_AS(orbitalforge::simulation::advance_semi_implicit_euler_step(
                        system, 1.0, 0.0),
                    std::invalid_argument);
}

TEST_CASE("semi-implicit Euler rejects a negative time step") {
  SystemState system = make_symmetric_two_body_system();

  REQUIRE_THROWS_AS(orbitalforge::simulation::advance_semi_implicit_euler_step(
                        system, 1.0, -0.1),
                    std::invalid_argument);
}

TEST_CASE("explicit Euler rejects an invalid gravitational constant") {
  SystemState system = make_symmetric_two_body_system();

  REQUIRE_THROWS_AS(
      orbitalforge::simulation::advance_explicit_euler_step(system, 0.0, 0.1),
      std::invalid_argument);

  REQUIRE_THROWS_AS(
      orbitalforge::simulation::advance_explicit_euler_step(system, -1.0, 0.1),
      std::invalid_argument);
}

TEST_CASE("semi-implicit Euler rejects an invalid gravitational constant") {
  SystemState system = make_symmetric_two_body_system();

  REQUIRE_THROWS_AS(orbitalforge::simulation::advance_semi_implicit_euler_step(
                        system, 0.0, 0.1),
                    std::invalid_argument);

  REQUIRE_THROWS_AS(orbitalforge::simulation::advance_semi_implicit_euler_step(
                        system, -1.0, 0.1),
                    std::invalid_argument);
}
