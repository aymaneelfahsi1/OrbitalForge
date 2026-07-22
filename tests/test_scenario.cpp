#include <limits>
#include <stdexcept>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/system_state.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"

namespace {

using orbitalforge::app::Scenario;
using orbitalforge::app::validate_scenario;
using orbitalforge::math::Vec3;
using orbitalforge::physics::Body;
using orbitalforge::physics::SystemState;
using orbitalforge::simulation::IntegratorKind;

[[nodiscard]] Scenario make_valid_scenario() {
  return Scenario{
      .name = "Two-body orbit",
      .gravitational_constant = 1.0,
      .softening = 0.0,
      .time_step = 0.01,
      .step_count = 1'000,
      .output_interval = 10,
      .integrator = IntegratorKind::velocity_verlet,
      .seed = 42,
      .initial_state =
          SystemState{
              .bodies{
                  Body{
                      "Primary",
                      1.0,
                      Vec3{-0.5, 0.0, 0.0},
                      Vec3{0.0, -0.5, 0.0},
                  },
                  Body{
                      "Secondary",
                      1.0,
                      Vec3{0.5, 0.0, 0.0},
                      Vec3{0.0, 0.5, 0.0},
                  },
              },
          },
  };
}

} // namespace

TEST_CASE("valid scenario is accepted") {
  const Scenario scenario = make_valid_scenario();

  REQUIRE_NOTHROW(validate_scenario(scenario));
}

TEST_CASE("scenario accepts zero softening") {
  Scenario scenario = make_valid_scenario();
  scenario.softening = 0.0;

  REQUIRE_NOTHROW(validate_scenario(scenario));
}

TEST_CASE("scenario accepts output interval larger than step count") {
  Scenario scenario = make_valid_scenario();

  scenario.step_count = 5;
  scenario.output_interval = 10;

  REQUIRE_NOTHROW(validate_scenario(scenario));
}

TEST_CASE("empty scenario name is rejected") {
  Scenario scenario = make_valid_scenario();
  scenario.name.clear();

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("scenario rejects zero gravitational constant") {
  Scenario scenario = make_valid_scenario();
  scenario.gravitational_constant = 0.0;

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("scenario rejects negative gravitational constant") {
  Scenario scenario = make_valid_scenario();
  scenario.gravitational_constant = -1.0;

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("non-finite gravitational constant is rejected") {
  SECTION("infinity") {
    Scenario scenario = make_valid_scenario();

    scenario.gravitational_constant = std::numeric_limits<double>::infinity();

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }

  SECTION("NaN") {
    Scenario scenario = make_valid_scenario();

    scenario.gravitational_constant = std::numeric_limits<double>::quiet_NaN();

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }
}

TEST_CASE("scenario rejects negative softening") {
  Scenario scenario = make_valid_scenario();
  scenario.softening = -0.01;

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("scenario rejects non-finite softening") {
  SECTION("infinity") {
    Scenario scenario = make_valid_scenario();
    scenario.softening = std::numeric_limits<double>::infinity();

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }

  SECTION("NaN") {
    Scenario scenario = make_valid_scenario();
    scenario.softening = std::numeric_limits<double>::quiet_NaN();

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }
}

TEST_CASE("scenario rejects zero time step") {
  Scenario scenario = make_valid_scenario();
  scenario.time_step = 0.0;

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("scenario rejects negative time step") {
  Scenario scenario = make_valid_scenario();
  scenario.time_step = -0.01;

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("non-finite time step is rejected") {
  SECTION("infinity") {
    Scenario scenario = make_valid_scenario();
    scenario.time_step = std::numeric_limits<double>::infinity();

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }

  SECTION("NaN") {
    Scenario scenario = make_valid_scenario();
    scenario.time_step = std::numeric_limits<double>::quiet_NaN();

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }
}

TEST_CASE("zero step count is rejected") {
  Scenario scenario = make_valid_scenario();
  scenario.step_count = 0;

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("scenario rejects zero output interval") {
  Scenario scenario = make_valid_scenario();
  scenario.output_interval = 0;

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("unknown integrator value is rejected") {
  Scenario scenario = make_valid_scenario();

  scenario.integrator = static_cast<IntegratorKind>(999);

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("scenario without bodies is rejected") {
  Scenario scenario = make_valid_scenario();
  scenario.initial_state.bodies.clear();

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("empty body name is rejected") {
  Scenario scenario = make_valid_scenario();
  scenario.initial_state.bodies[0].name.clear();

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("duplicate body names are rejected") {
  Scenario scenario = make_valid_scenario();

  scenario.initial_state.bodies[1].name = scenario.initial_state.bodies[0].name;

  REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
}

TEST_CASE("non-finite body mass is rejected") {
  SECTION("infinity") {
    Scenario scenario = make_valid_scenario();

    scenario.initial_state.bodies[0].mass =
        std::numeric_limits<double>::infinity();

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }

  SECTION("NaN") {
    Scenario scenario = make_valid_scenario();

    scenario.initial_state.bodies[0].mass =
        std::numeric_limits<double>::quiet_NaN();

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }
}

TEST_CASE("non-positive body mass is rejected after mutation") {
  SECTION("zero") {
    Scenario scenario = make_valid_scenario();
    scenario.initial_state.bodies[0].mass = 0.0;

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }

  SECTION("negative") {
    Scenario scenario = make_valid_scenario();
    scenario.initial_state.bodies[0].mass = -1.0;

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }
}

TEST_CASE("non-finite body position is rejected") {
  SECTION("infinite x coordinate") {
    Scenario scenario = make_valid_scenario();

    scenario.initial_state.bodies[0].position =
        Vec3{std::numeric_limits<double>::infinity(), 0.0, 0.0};

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }

  SECTION("NaN z coordinate") {
    Scenario scenario = make_valid_scenario();

    scenario.initial_state.bodies[0].position =
        Vec3{0.0, 0.0, std::numeric_limits<double>::quiet_NaN()};

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }
}

TEST_CASE("non-finite body velocity is rejected") {
  SECTION("infinite y coordinate") {
    Scenario scenario = make_valid_scenario();

    scenario.initial_state.bodies[0].velocity =
        Vec3{0.0, std::numeric_limits<double>::infinity(), 0.0};

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }

  SECTION("NaN x coordinate") {
    Scenario scenario = make_valid_scenario();

    scenario.initial_state.bodies[0].velocity =
        Vec3{std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0};

    REQUIRE_THROWS_AS(validate_scenario(scenario), std::invalid_argument);
  }
}
