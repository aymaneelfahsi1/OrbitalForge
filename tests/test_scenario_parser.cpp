#include <sstream>
#include <string>
#include <variant>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "orbitalforge/app/parse_error.hpp"
#include "orbitalforge/app/scenario.hpp"
#include "orbitalforge/app/scenario_parser.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"

namespace {

using Catch::Approx;
using orbitalforge::app::parse_scenario;
using orbitalforge::app::ParseError;
using orbitalforge::app::Scenario;
using orbitalforge::app::ScenarioParseResult;
using orbitalforge::simulation::IntegratorKind;

[[nodiscard]] std::string valid_scenario_text() {
  return R"(
# OrbitalForge two-body scenario

name = Two-body orbit
gravitational_constant = 1.0
softening = 0.0
time_step = 0.01
step_count = 1000
output_interval = 10
integrator = velocity-verlet
seed = 42

body = Primary,1.0,-0.5,0.0,0.0,0.0,-0.5,0.0
body = Secondary,1.0,0.5,0.0,0.0,0.0,0.5,0.0
)";
}

[[nodiscard]] ScenarioParseResult parse_text(const std::string &text) {
  std::istringstream input{text};
  return parse_scenario(input);
}

[[nodiscard]] const ParseError &
require_parse_error(const ScenarioParseResult &result) {
  REQUIRE(std::holds_alternative<ParseError>(result));
  return std::get<ParseError>(result);
}

} // namespace

TEST_CASE("valid scenario text is parsed") {
  const ScenarioParseResult result = parse_text(valid_scenario_text());

  REQUIRE(std::holds_alternative<Scenario>(result));

  const Scenario &scenario = std::get<Scenario>(result);

  REQUIRE(scenario.name == "Two-body orbit");
  REQUIRE(scenario.gravitational_constant == Approx(1.0));
  REQUIRE(scenario.softening == Approx(0.0));
  REQUIRE(scenario.time_step == Approx(0.01));
  REQUIRE(scenario.step_count == 1000);
  REQUIRE(scenario.output_interval == 10);
  REQUIRE(scenario.integrator == IntegratorKind::velocity_verlet);
  REQUIRE(scenario.seed == 42);

  REQUIRE(scenario.initial_state.bodies.size() == 2);

  REQUIRE(scenario.initial_state.bodies[0].name == "Primary");
  REQUIRE(scenario.initial_state.bodies[0].mass == Approx(1.0));
  REQUIRE(scenario.initial_state.bodies[0].position.x() == Approx(-0.5));
  REQUIRE(scenario.initial_state.bodies[0].velocity.y() == Approx(-0.5));

  REQUIRE(scenario.initial_state.bodies[1].name == "Secondary");
  REQUIRE(scenario.initial_state.bodies[1].mass == Approx(1.0));
  REQUIRE(scenario.initial_state.bodies[1].position.x() == Approx(0.5));
  REQUIRE(scenario.initial_state.bodies[1].velocity.y() == Approx(0.5));
}

TEST_CASE("parser ignores blank lines and comments") {
  const ScenarioParseResult result = parse_text(valid_scenario_text());

  REQUIRE(std::holds_alternative<Scenario>(result));
}

TEST_CASE("parser supports inline comments") {
  const std::string text = R"(
name = Comment test
gravitational_constant = 1.0 # normalized gravity
softening = 0.0
time_step = 0.01
step_count = 10
output_interval = 1
integrator = explicit-euler
seed = 7
body = Body,1.0,0,0,0,1,0,0 # moving body
)";

  const ScenarioParseResult result = parse_text(text);

  REQUIRE(std::holds_alternative<Scenario>(result));

  const Scenario &scenario = std::get<Scenario>(result);

  REQUIRE(scenario.gravitational_constant == Approx(1.0));
  REQUIRE(scenario.initial_state.bodies.size() == 1);
}

TEST_CASE("line without equals sign is rejected") {
  const std::string text = R"(
name Two-body orbit
)";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.line_number == 2);
  REQUIRE(error.message == "expected a key-value pair separated by '='");
}

TEST_CASE("empty value is rejected") {
  const std::string text = R"(
name =
)";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.line_number == 2);
  REQUIRE(error.message == "value for 'name' must not be empty");
}

TEST_CASE("unknown key is rejected") {
  const std::string text = R"(
name = Test
gravity = 1.0
)";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.line_number == 3);
  REQUIRE(error.message == "unknown key: gravity");
}

TEST_CASE("duplicate single-value key is rejected") {
  const std::string text = R"(
name = First name
name = Second name
)";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.line_number == 3);
  REQUIRE(error.message == "duplicate key: name");
}

TEST_CASE("multiple body keys are allowed") {
  const ScenarioParseResult result = parse_text(valid_scenario_text());

  REQUIRE(std::holds_alternative<Scenario>(result));

  const Scenario &scenario = std::get<Scenario>(result);

  REQUIRE(scenario.initial_state.bodies.size() == 2);
}

TEST_CASE("invalid gravitational constant syntax is rejected") {
  std::string text = valid_scenario_text();

  const std::string original = "gravitational_constant = 1.0";
  const std::string replacement = "gravitational_constant = gravity";

  text.replace(text.find(original), original.size(), replacement);

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.message == "invalid gravitational constant");
}

TEST_CASE("numeric values with trailing characters are rejected") {
  std::string text = valid_scenario_text();

  const std::string original = "time_step = 0.01";
  const std::string replacement = "time_step = 0.01seconds";

  text.replace(text.find(original), original.size(), replacement);

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.message == "invalid time step");
}

TEST_CASE("negative unsigned step count is rejected") {
  std::string text = valid_scenario_text();

  const std::string original = "step_count = 1000";
  const std::string replacement = "step_count = -1";

  text.replace(text.find(original), original.size(), replacement);

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.message == "invalid step count");
}

TEST_CASE("unknown integrator is rejected") {
  std::string text = valid_scenario_text();

  const std::string original = "integrator = velocity-verlet";
  const std::string replacement = "integrator = teleport";

  text.replace(text.find(original), original.size(), replacement);

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.message == "unknown integrator: teleport");
}

TEST_CASE("body with wrong field count is rejected") {
  std::string text = valid_scenario_text();

  text += "body = Broken,1.0,0,0\n";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.message == "body requires 8 comma-separated fields: "
                           "name,mass,px,py,pz,vx,vy,vz");
}

TEST_CASE("body with invalid mass is rejected") {
  std::string text = valid_scenario_text();

  text += "body = Broken,no-mass,0,0,0,0,0,0\n";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.message == "invalid body mass");
}

TEST_CASE("body with invalid position is rejected") {
  std::string text = valid_scenario_text();

  text += "body = Broken,1.0,x,0,0,0,0,0\n";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.message == "invalid body position");
}

TEST_CASE("body with invalid velocity is rejected") {
  std::string text = valid_scenario_text();

  text += "body = Broken,1.0,0,0,0,fast,0,0\n";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.message == "invalid body velocity");
}

TEST_CASE("missing required key is rejected") {
  const std::string text = R"(
name = Missing fields
)";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.line_number == 0);
  REQUIRE(error.message == "missing required key: gravitational_constant");
}

TEST_CASE("scenario validation failures become parse errors") {
  std::string text = valid_scenario_text();

  const std::string original = "time_step = 0.01";
  const std::string replacement = "time_step = 0.0";

  text.replace(text.find(original), original.size(), replacement);

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.line_number == 0);
  REQUIRE(error.message == "time step must be positive and finite");
}

TEST_CASE("scenario parser rejects duplicate body names") {
  std::string text = valid_scenario_text();

  text += "body = Primary,1.0,1,0,0,0,1,0\n";

  const ScenarioParseResult result = parse_text(text);
  const ParseError &error = require_parse_error(result);

  REQUIRE(error.line_number == 0);
  REQUIRE(error.message == "duplicate body name: Primary");
}
