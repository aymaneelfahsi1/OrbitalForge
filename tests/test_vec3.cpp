#include "orbitalforge/math/vec3.hpp"
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

using orbitalforge::math::Vec3;

TEST_CASE("default vector is the zero vector") {
  const Vec3 vector;

  REQUIRE(vector.x() == 0.0);
  REQUIRE(vector.y() == 0.0);
  REQUIRE(vector.z() == 0.0);
}
TEST_CASE("vector stores three coordinates") {
  const Vec3 vector{1.0, 2.0, 3.0};

  REQUIRE(vector.x() == 1.0);
  REQUIRE(vector.y() == 2.0);
  REQUIRE(vector.z() == 3.0);
}

TEST_CASE("vector computes its squared norm") {
  const Vec3 vector{2.0, 3.0, 6.0};

  REQUIRE(vector.squared_norm() == 49.0);
}

TEST_CASE("vector computes its norm") {
  const Vec3 vector{3.0, 4.0, 0.0};

  REQUIRE(vector.norm() == Catch::Approx(5.0));
}

TEST_CASE("vectors can be added") {
  const Vec3 first{1.0, 2.0, 3.0};
  const Vec3 second{4.0, 5.0, 6.0};

  const Vec3 result = first + second;

  REQUIRE(result == Vec3{5.0, 7.0, 9.0});
}

TEST_CASE("vectors can be substracted") {
  const Vec3 first{1.0, 2.0, 3.0};
  const Vec3 second{4.0, 5.0, 6.0};

  const Vec3 result = first - second;

  REQUIRE(result == Vec3{-3.0, -3.0, -3.0});
}

TEST_CASE("vectors can be multiplied by scalar") {
  const Vec3 vector{1.0, 2.0, 3.0};

  REQUIRE(2 * vector == Vec3{2, 4, 6});
  REQUIRE(vector * 2 == Vec3{2, 4, 6});
}

TEST_CASE("vector supports compound addition") {
  Vec3 vector{1.0, 2.0, 3.0};

  vector += Vec3{4.0, 5.0, 6.0};

  REQUIRE(vector == Vec3{5.0, 7.0, 9.0});
}

TEST_CASE("dot product of perpendicular vectors is zero") {
  const Vec3 x_axis{1.0, 0.0, 0.0};
  const Vec3 y_axis{0.0, 1.0, 0.0};

  REQUIRE(x_axis.dot(y_axis) == 0.0);
}

TEST_CASE("vector can be divided by scalar") {
  const Vec3 vector{2.0, 4.0, 6.0};

  REQUIRE(vector / 2.0 == Vec3{1.0, 2.0, 3.0});
}

TEST_CASE("vector can be normalized") {
  const Vec3 vector{3.0, 4.0, 0.0};

  const Vec3 unit = vector.normalized();

  REQUIRE(unit.x() == Catch::Approx(0.6));
  REQUIRE(unit.y() == Catch::Approx(0.8));
  REQUIRE(unit.z() == Catch::Approx(0.0));
  REQUIRE(unit.norm() == Catch::Approx(1.0));
}

TEST_CASE("normalizing zero vector throws") {
  const Vec3 zero;

  REQUIRE_THROWS_AS(zero.normalized(), std::domain_error);
}

TEST_CASE("vector supports compound subtraction") {
  Vec3 vector{5.0, 7.0, 9.0};

  vector -= Vec3{1.0, 2.0, 3.0};

  REQUIRE(vector == Vec3{4.0, 5.0, 6.0});
}

TEST_CASE("vector supports compound scalar multiplication") {
  Vec3 vector{1.0, 2.0, 3.0};

  vector *= 2.0;

  REQUIRE(vector == Vec3{2.0, 4.0, 6.0});
}

TEST_CASE("vector supports compound scalar division") {
  Vec3 vector{2.0, 4.0, 6.0};

  vector /= 2.0;

  REQUIRE(vector == Vec3{1.0, 2.0, 3.0});
}

TEST_CASE("dot product computes the expected value") {
  const Vec3 first{1.0, 2.0, 3.0};
  const Vec3 second{4.0, 5.0, 6.0};

  REQUIRE(first.dot(second) == Catch::Approx(32.0));
}

TEST_CASE("normalizing a negative direction preserves direction") {
  const Vec3 vector{-3.0, 0.0, 0.0};

  const Vec3 unit = vector.normalized();

  REQUIRE(unit.x() == Catch::Approx(-1.0));
  REQUIRE(unit.y() == Catch::Approx(0.0));
  REQUIRE(unit.z() == Catch::Approx(0.0));
  REQUIRE(unit.norm() == Catch::Approx(1.0));
}

TEST_CASE("vector rejects compound division by zero") {
  Vec3 vector{1.0, 2.0, 3.0};

  REQUIRE_THROWS_AS(vector /= 0.0, std::domain_error);
}

TEST_CASE("vector division by zero throws") {
  const Vec3 vector{1.0, 2.0, 3.0};

  REQUIRE_THROWS_AS(vector / 0.0, std::domain_error);
}

TEST_CASE("vector arithmetic does not modify its operands") {
  const Vec3 first{5.0, 7.0, 9.0};
  const Vec3 second{1.0, 2.0, 3.0};

  const Vec3 result = first - second;

  REQUIRE(result == Vec3{4.0, 5.0, 6.0});
  REQUIRE(first == Vec3{5.0, 7.0, 9.0});
  REQUIRE(second == Vec3{1.0, 2.0, 3.0});
}

TEST_CASE("vector can be written to an output stream") {
  const Vec3 vector{1.0, 2.0, 3.0};
  std::ostringstream output;

  output << vector;

  REQUIRE(output.str() == "(1, 2, 3)");
}