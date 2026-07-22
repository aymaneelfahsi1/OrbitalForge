#include "orbitalforge/app/scenario_parser.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_set>
#include <utility>
#include <vector>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/simulation/integrator_kind.hpp"

namespace orbitalforge::app {

namespace {

using math::Vec3;
using physics::Body;
using simulation::IntegratorKind;
using simulation::parse_integrator_kind;

[[nodiscard]] std::string_view trim(std::string_view text) noexcept {
  const std::size_t first = text.find_first_not_of(" \t\r\n");

  if (first == std::string_view::npos) {
    return {};
  }

  const std::size_t last = text.find_last_not_of(" \t\r\n");

  return text.substr(first, last - first + 1);
}

[[nodiscard]] std::string remove_comment(std::string_view line) {
  const std::size_t comment_position = line.find('#');

  if (comment_position != std::string_view::npos) {
    line = line.substr(0, comment_position);
  }

  return std::string{trim(line)};
}

[[nodiscard]] std::optional<double>
parse_double(std::string_view text) noexcept {
  text = trim(text);

  if (text.empty()) {
    return std::nullopt;
  }

  double value{};

  const char *first = text.data();
  const char *last = first + text.size();

  const auto [pointer, error] = std::from_chars(first, last, value);

  if (error != std::errc{} || pointer != last) {
    return std::nullopt;
  }

  return value;
}

template <typename Integer>
[[nodiscard]] std::optional<Integer>
parse_integer(std::string_view text) noexcept {
  text = trim(text);

  if (text.empty()) {
    return std::nullopt;
  }

  Integer value{};

  const char *first = text.data();
  const char *last = first + text.size();

  const auto [pointer, error] = std::from_chars(first, last, value);

  if (error != std::errc{} || pointer != last) {
    return std::nullopt;
  }

  return value;
}

[[nodiscard]] std::vector<std::string_view> split(std::string_view text,
                                                  char delimiter) {
  std::vector<std::string_view> values;

  std::size_t start = 0;

  while (start <= text.size()) {
    const std::size_t delimiter_position = text.find(delimiter, start);

    if (delimiter_position == std::string_view::npos) {
      values.push_back(trim(text.substr(start)));
      break;
    }

    values.push_back(trim(text.substr(start, delimiter_position - start)));

    start = delimiter_position + 1;
  }

  return values;
}

[[nodiscard]] ScenarioParseResult make_error(std::size_t line_number,
                                             std::string message) {
  return ParseError{
      .line_number = line_number,
      .message = std::move(message),
  };
}

[[nodiscard]] std::variant<Body, ParseError>
parse_body(std::string_view value, std::size_t line_number) {
  const std::vector<std::string_view> fields = split(value, ',');

  if (fields.size() != 8) {
    return ParseError{
        .line_number = line_number,
        .message = "body requires 8 comma-separated fields: "
                   "name,mass,px,py,pz,vx,vy,vz",
    };
  }

  if (fields[0].empty()) {
    return ParseError{
        .line_number = line_number,
        .message = "body name must not be empty",
    };
  }

  const std::optional<double> mass = parse_double(fields[1]);
  const std::optional<double> position_x = parse_double(fields[2]);
  const std::optional<double> position_y = parse_double(fields[3]);
  const std::optional<double> position_z = parse_double(fields[4]);
  const std::optional<double> velocity_x = parse_double(fields[5]);
  const std::optional<double> velocity_y = parse_double(fields[6]);
  const std::optional<double> velocity_z = parse_double(fields[7]);

  if (!mass.has_value()) {
    return ParseError{
        .line_number = line_number,
        .message = "invalid body mass",
    };
  }

  if (!position_x.has_value() || !position_y.has_value() ||
      !position_z.has_value()) {
    return ParseError{
        .line_number = line_number,
        .message = "invalid body position",
    };
  }

  if (!velocity_x.has_value() || !velocity_y.has_value() ||
      !velocity_z.has_value()) {
    return ParseError{
        .line_number = line_number,
        .message = "invalid body velocity",
    };
  }

  try {
    return Body{
        std::string{fields[0]},
        *mass,
        Vec3{*position_x, *position_y, *position_z},
        Vec3{*velocity_x, *velocity_y, *velocity_z},
    };
  } catch (const std::invalid_argument &error) {
    return ParseError{
        .line_number = line_number,
        .message = error.what(),
    };
  }
}

} // namespace

ScenarioParseResult parse_scenario(std::istream &input) {
  Scenario scenario{};

  bool has_name = false;
  bool has_gravitational_constant = false;
  bool has_softening = false;
  bool has_time_step = false;
  bool has_step_count = false;
  bool has_output_interval = false;
  bool has_integrator = false;
  bool has_seed = false;

  std::unordered_set<std::string> seen_single_value_keys;

  std::string raw_line;
  std::size_t line_number = 0;

  while (std::getline(input, raw_line)) {
    ++line_number;

    const std::string line = remove_comment(raw_line);

    if (line.empty()) {
      continue;
    }

    const std::size_t equals_position = line.find('=');

    if (equals_position == std::string::npos) {
      return make_error(line_number,
                        "expected a key-value pair separated by '='");
    }

    const std::string key{
        trim(std::string_view{line}.substr(0, equals_position))};

    const std::string_view value =
        trim(std::string_view{line}.substr(equals_position + 1));

    if (key.empty()) {
      return make_error(line_number, "key must not be empty");
    }

    if (value.empty()) {
      return make_error(line_number,
                        "value for '" + key + "' must not be empty");
    }

    if (key == "body") {
      std::variant<Body, ParseError> body_result =
          parse_body(value, line_number);

      if (const auto *error = std::get_if<ParseError>(&body_result)) {
        return *error;
      }

      scenario.initial_state.bodies.push_back(
          std::get<Body>(std::move(body_result)));

      continue;
    }

    const bool inserted = seen_single_value_keys.insert(key).second;

    if (!inserted) {
      return make_error(line_number, "duplicate key: " + key);
    }

    if (key == "name") {
      scenario.name = std::string{value};
      has_name = true;
      continue;
    }

    if (key == "gravitational_constant") {
      const std::optional<double> parsed = parse_double(value);

      if (!parsed.has_value()) {
        return make_error(line_number, "invalid gravitational constant");
      }

      scenario.gravitational_constant = *parsed;
      has_gravitational_constant = true;
      continue;
    }

    if (key == "softening") {
      const std::optional<double> parsed = parse_double(value);

      if (!parsed.has_value()) {
        return make_error(line_number, "invalid softening value");
      }

      scenario.softening = *parsed;
      has_softening = true;
      continue;
    }

    if (key == "time_step") {
      const std::optional<double> parsed = parse_double(value);

      if (!parsed.has_value()) {
        return make_error(line_number, "invalid time step");
      }

      scenario.time_step = *parsed;
      has_time_step = true;
      continue;
    }

    if (key == "step_count") {
      const std::optional<std::size_t> parsed =
          parse_integer<std::size_t>(value);

      if (!parsed.has_value()) {
        return make_error(line_number, "invalid step count");
      }

      scenario.step_count = *parsed;
      has_step_count = true;
      continue;
    }

    if (key == "output_interval") {
      const std::optional<std::size_t> parsed =
          parse_integer<std::size_t>(value);

      if (!parsed.has_value()) {
        return make_error(line_number, "invalid output interval");
      }

      scenario.output_interval = *parsed;
      has_output_interval = true;
      continue;
    }

    if (key == "integrator") {
      const std::optional<IntegratorKind> parsed = parse_integrator_kind(value);

      if (!parsed.has_value()) {
        return make_error(line_number,
                          "unknown integrator: " + std::string{value});
      }

      scenario.integrator = *parsed;
      has_integrator = true;
      continue;
    }

    if (key == "seed") {
      const std::optional<std::uint64_t> parsed =
          parse_integer<std::uint64_t>(value);

      if (!parsed.has_value()) {
        return make_error(line_number, "invalid seed");
      }

      scenario.seed = *parsed;
      has_seed = true;
      continue;
    }

    return make_error(line_number, "unknown key: " + key);
  }

  if (input.bad()) {
    return make_error(line_number, "failed while reading input");
  }

  if (!has_name) {
    return make_error(0, "missing required key: name");
  }

  if (!has_gravitational_constant) {
    return make_error(0, "missing required key: gravitational_constant");
  }

  if (!has_softening) {
    return make_error(0, "missing required key: softening");
  }

  if (!has_time_step) {
    return make_error(0, "missing required key: time_step");
  }

  if (!has_step_count) {
    return make_error(0, "missing required key: step_count");
  }

  if (!has_output_interval) {
    return make_error(0, "missing required key: output_interval");
  }

  if (!has_integrator) {
    return make_error(0, "missing required key: integrator");
  }

  if (!has_seed) {
    return make_error(0, "missing required key: seed");
  }

  try {
    validate_scenario(scenario);
  } catch (const std::invalid_argument &error) {
    return make_error(0, error.what());
  }

  return scenario;
}

ScenarioParseResult parse_scenario_file(const std::filesystem::path &path) {
  std::ifstream input{path};

  if (!input.is_open()) {
    return make_error(0, "could not open scenario file: " + path.string());
  }

  return parse_scenario(input);
}

} // namespace orbitalforge::app