#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/system_state.hpp"

#include <catch2/catch_test_macros.hpp>

using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::SystemState;

TEST_CASE("system state is empty by default") {
  const SystemState system;

  REQUIRE(system.bodies.empty());
}

TEST_CASE("system state stores multiple bodies") {
  SystemState system{
      .bodies{Body{"Earth", 5.972e24, Vec3{}, Vec3{}},
              Body{"Moon", 7.342e22, Vec3{384.4e6, 0.0, 0.0}, Vec3{}}}};

  REQUIRE(system.bodies.size() == 2);
  REQUIRE(system.bodies[0].name == "Earth");
  REQUIRE(system.bodies[1].name == "Moon");
}