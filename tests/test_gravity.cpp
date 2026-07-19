#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/gravity.hpp"

using Catch::Approx;
using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::gravitational_acceleration;

TEST_CASE("gravity points toward the origin") {
  constexpr double earth_mu = 3.986004418e14;

  const Vec3 position{7.0e6, 0.0, 0.0};

  const Vec3 acceleration = gravitational_acceleration(position, earth_mu);

  REQUIRE(acceleration.x() < 0.0);
  REQUIRE(acceleration.y() == Approx(0.0));
  REQUIRE(acceleration.z() == Approx(0.0));
}

TEST_CASE("gravity has expected magnitude near low Earth orbit") {
  constexpr double earth_mu = 3.986004418e14;

  const Vec3 position{7.0e6, 0.0, 0.0};

  const Vec3 acceleration = gravitational_acceleration(position, earth_mu);

  REQUIRE(acceleration.norm() == Approx(8.1347).margin(0.001));
}

TEST_CASE("gravity rejects the origin") {
  constexpr double earth_mu = 3.986004418e14;

  REQUIRE_THROWS_AS(gravitational_acceleration(Vec3{}, earth_mu),
                    std::domain_error);
}

TEST_CASE("gravity rejects invalid gravitational parameter") {
  const Vec3 position{7.0e6, 0.0, 0.0};

  REQUIRE_THROWS_AS(gravitational_acceleration(position, 0.0),
                    std::invalid_argument);
}

TEST_CASE("source body accelerates target toward itself") {
  constexpr double gravitational_constant = 1.0;

  const Body target{"Target", 1.0, Vec3{.0, .0, .0}, Vec3{}};

  const Body source{"Source", 2.0, Vec3{2.0, .0, .0}, Vec3{}};

  const Vec3 acceleration =
      gravitational_acceleration(target, source, gravitational_constant);

  REQUIRE(acceleration.x() == Catch::Approx(.5));
  REQUIRE(acceleration.y() == Catch::Approx(.0));
  REQUIRE(acceleration.z() == Catch::Approx(.0));
}

TEST_CASE("gravity acceleration preserves spatial direction") {
  constexpr double gravitational_constant = 1.0;

  const Body target{"Target", 1.0, Vec3{0.0, 0.0, 0.0}, Vec3{}};

  const Body source{"Source", 4.0, Vec3{1.0, 2.0, 3.0}, Vec3{}};

  const Vec3 displacement = source.position - target.position;

  const Vec3 acceleration =
      gravitational_acceleration(target, source, gravitational_constant);

  REQUIRE(acceleration.cross(displacement).norm() ==
          Catch::Approx(0.0).margin(1e-12));

  REQUIRE(acceleration.dot(displacement) > 0.0);
}