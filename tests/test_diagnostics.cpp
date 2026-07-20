#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <stdexcept>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/diagnostics.hpp"
#include "orbitalforge/physics/system_state.hpp"

using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::center_of_mass;
using orbitalforge::physics::kinetic_energy;
using orbitalforge::physics::potential_energy;
using orbitalforge::physics::SystemState;
using orbitalforge::physics::total_energy;
using orbitalforge::physics::total_momentum;

TEST_CASE("empty system has zero kinetic energy") {
  const SystemState system{};

  REQUIRE(kinetic_energy(system) == Catch::Approx(0.0));
}

TEST_CASE("kinetic energy sums contributions from every body") {
  const SystemState system{
      .bodies{
          Body{
              "First",
              2.0,
              Vec3{},
              Vec3{3.0, 0.0, 0.0},
          },
          Body{
              "Second",
              4.0,
              Vec3{},
              Vec3{0.0, 2.0, 0.0},
          },
      },
  };

  REQUIRE(kinetic_energy(system) == Catch::Approx(17.0));
}

TEST_CASE("stationary bodies have zero kinetic energy") {
  const SystemState system{
      .bodies{
          Body{
              "First",
              2.0,
              Vec3{},
              Vec3{},
          },
          Body{
              "Second",
              4.0,
              Vec3{1.0, 0.0, 0.0},
              Vec3{},
          },
      },
  };

  REQUIRE(kinetic_energy(system) == Catch::Approx(0.0));
}

TEST_CASE("equal and opposite momenta cancel") {
  const SystemState system{
      .bodies{
          Body{
              "First",
              2.0,
              Vec3{},
              Vec3{3.0, 0.0, 0.0},
          },
          Body{
              "Second",
              3.0,
              Vec3{},
              Vec3{-2.0, 0.0, 0.0},
          },
      },
  };

  REQUIRE(total_momentum(system) == Vec3{});
}

TEST_CASE("total momentum sums all momentum components") {
  const SystemState system{
      .bodies{
          Body{
              "First",
              2.0,
              Vec3{},
              Vec3{1.0, 2.0, 3.0},
          },
          Body{
              "Second",
              3.0,
              Vec3{},
              Vec3{-1.0, 1.0, 2.0},
          },
      },
  };

  const Vec3 momentum = total_momentum(system);

  REQUIRE(momentum.x() == Catch::Approx(-1.0));
  REQUIRE(momentum.y() == Catch::Approx(7.0));
  REQUIRE(momentum.z() == Catch::Approx(12.0));
}

TEST_CASE("center of mass uses mass weighted positions") {
  const SystemState system{
      .bodies{
          Body{
              "Light",
              1.0,
              Vec3{0.0, 0.0, 0.0},
              Vec3{},
          },
          Body{
              "Heavy",
              3.0,
              Vec3{4.0, 0.0, 0.0},
              Vec3{},
          },
      },
  };

  const Vec3 result = center_of_mass(system);

  REQUIRE(result.x() == Catch::Approx(3.0));
  REQUIRE(result.y() == Catch::Approx(0.0));
  REQUIRE(result.z() == Catch::Approx(0.0));
}

TEST_CASE("single body is located at the center of mass") {
  const SystemState system{
      .bodies{
          Body{
              "Only body",
              5.0,
              Vec3{2.0, -4.0, 7.0},
              Vec3{},
          },
      },
  };

  REQUIRE(center_of_mass(system) == Vec3{2.0, -4.0, 7.0});
}

TEST_CASE("center of mass is undefined for an empty system") {
  const SystemState system{};

  REQUIRE_THROWS_AS(center_of_mass(system), std::domain_error);
}

TEST_CASE("two body potential energy is negative") {
  constexpr double gravitational_constant = 1.0;

  const SystemState system{
      .bodies{
          Body{
              "First",
              2.0,
              Vec3{0.0, 0.0, 0.0},
              Vec3{},
          },
          Body{
              "Second",
              4.0,
              Vec3{2.0, 0.0, 0.0},
              Vec3{},
          },
      },
  };

  REQUIRE(potential_energy(system, gravitational_constant) ==
          Catch::Approx(-4.0));
}

TEST_CASE("potential energy counts each pair exactly once") {
  constexpr double gravitational_constant = 1.0;

  const SystemState system{
      .bodies{
          Body{
              "First",
              1.0,
              Vec3{0.0, 0.0, 0.0},
              Vec3{},
          },
          Body{
              "Second",
              2.0,
              Vec3{1.0, 0.0, 0.0},
              Vec3{},
          },
          Body{
              "Third",
              3.0,
              Vec3{3.0, 0.0, 0.0},
              Vec3{},
          },
      },
  };

  REQUIRE(potential_energy(system, gravitational_constant) ==
          Catch::Approx(-6.0));
}

TEST_CASE("empty and single body systems have zero potential energy") {
  constexpr double gravitational_constant = 1.0;

  const SystemState empty_system{};

  const SystemState single_body_system{
      .bodies{
          Body{
              "Only body",
              1.0,
              Vec3{},
              Vec3{},
          },
      },
  };

  REQUIRE(potential_energy(empty_system, gravitational_constant) ==
          Catch::Approx(0.0));

  REQUIRE(potential_energy(single_body_system, gravitational_constant) ==
          Catch::Approx(0.0));
}

TEST_CASE("potential energy rejects invalid gravitational constants") {
  const SystemState system{};

  REQUIRE_THROWS_AS(potential_energy(system, 0.0), std::invalid_argument);

  REQUIRE_THROWS_AS(potential_energy(system, -1.0), std::invalid_argument);
}

TEST_CASE("potential energy is undefined for coincident bodies") {
  constexpr double gravitational_constant = 1.0;

  const SystemState system{
      .bodies{
          Body{
              "First",
              1.0,
              Vec3{2.0, 3.0, 4.0},
              Vec3{},
          },
          Body{
              "Second",
              2.0,
              Vec3{2.0, 3.0, 4.0},
              Vec3{},
          },
      },
  };

  REQUIRE_THROWS_AS(potential_energy(system, gravitational_constant),
                    std::domain_error);
}

TEST_CASE("total energy combines kinetic and potential energy") {
  constexpr double gravitational_constant = 1.0;

  const SystemState system{
      .bodies{
          Body{
              "First",
              2.0,
              Vec3{0.0, 0.0, 0.0},
              Vec3{1.0, 0.0, 0.0},
          },
          Body{
              "Second",
              2.0,
              Vec3{2.0, 0.0, 0.0},
              Vec3{-1.0, 0.0, 0.0},
          },
      },
  };

  REQUIRE(total_energy(system, gravitational_constant) == Catch::Approx(0.0));
}