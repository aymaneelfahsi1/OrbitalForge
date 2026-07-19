#include <catch2/catch_test_macros.hpp>

#include <stdexcept>
#include <string>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"

using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;

TEST_CASE("body stores its physical state") {
  const Body body{"Satellite", 100.0, Vec3{1.0, 2.0, 3.0}, Vec3{4.0, 5.0, 6.0}};

  REQUIRE(body.name == "Satellite");
  REQUIRE(body.mass == 100.0);
  REQUIRE(body.position == Vec3{1.0, 2.0, 3.0});
  REQUIRE(body.velocity == Vec3{4.0, 5.0, 6.0});
}

TEST_CASE("body rejects zero mass") {
  REQUIRE_THROWS_AS((Body{"Impossible", 0.0, Vec3{}, Vec3{}}),
                    std::invalid_argument);
}

TEST_CASE("body rejects negative mass") {

  REQUIRE_THROWS_AS((Body{"Impossible", -50.0, Vec3{}, Vec3{}}),
                    std::invalid_argument);
}