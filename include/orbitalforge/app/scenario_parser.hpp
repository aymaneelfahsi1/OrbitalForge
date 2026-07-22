#pragma once

#include <filesystem>
#include <istream>
#include <variant>

#include "orbitalforge/app/parse_error.hpp"
#include "orbitalforge/app/scenario.hpp"

namespace orbitalforge::app {

using ScenarioParseResult = std::variant<Scenario, ParseError>;

[[nodiscard]] ScenarioParseResult parse_scenario(std::istream &input);

[[nodiscard]] ScenarioParseResult
parse_scenario_file(const std::filesystem::path &path);

} // namespace orbitalforge::app