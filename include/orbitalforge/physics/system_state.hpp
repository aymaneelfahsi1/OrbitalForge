#pragma once

#include <vector>

#include "orbitalforge/physics/body.hpp"

namespace orbitalforge::physics {

struct SystemState {
  std::vector<Body> bodies;
};

} // namespace orbitalforge::physics