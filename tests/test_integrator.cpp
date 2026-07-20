#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/integrator.hpp"
#include "orbitalforge/physics/state.hpp"

using Catch::Approx;
using orbitalforge::math::Vec3;
using orbitalforge::physics::explicit_euler_step;
using orbitalforge::physics::semi_implicit_euler_step;
using orbitalforge::physics::State;

TEST_CASE("explicit Euler uses current velocity for position") {
  const State state{Vec3{10.0, 0.0, 0.0}, Vec3{2.0, 0.0, 0.0}};

  const Vec3 acceleration{-1.0, 0.0, 0.0};

  const State next = explicit_euler_step(state, acceleration, 1.0);

  REQUIRE(next.position == Vec3{12.0, 0.0, 0.0});
  REQUIRE(next.velocity == Vec3{1.0, 0.0, 0.0});
}

TEST_CASE("semi implicit Euler uses updated velocity for position") {
  const State state{Vec3{10.0, 0.0, 0.0}, Vec3{2.0, 0.0, 0.0}};

  const Vec3 acceleration{-1.0, 0.0, 0.0};

  const State next = semi_implicit_euler_step(state, acceleration, 1.0);

  REQUIRE(next.velocity == Vec3{1.0, 0.0, 0.0});
  REQUIRE(next.position == Vec3{11.0, 0.0, 0.0});
}

TEST_CASE("integrators reject zero time step") {
  const State state{Vec3{}, Vec3{}};

  REQUIRE_THROWS_AS(explicit_euler_step(state, Vec3{}, 0.0),
                    std::invalid_argument);

  REQUIRE_THROWS_AS(semi_implicit_euler_step(state, Vec3{}, 0.0),
                    std::invalid_argument);
}

TEST_CASE("integrators reject negative time step") {
  const State state{Vec3{}, Vec3{}};

  REQUIRE_THROWS_AS(explicit_euler_step(state, Vec3{}, -1.0),
                    std::invalid_argument);
}
