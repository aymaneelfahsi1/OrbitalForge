#pragma once

#include "orbitalforge/math/vec3.hpp"

namespace orbitalforge::physics {

struct State {
  math::Vec3 position;
  math::Vec3 velocity;
};

} // namespace orbitalforge::physics