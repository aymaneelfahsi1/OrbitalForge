#pragma once

#include "orbitalforge/math/vec3.hpp"

namespace orbitalforge::physics {

[[nodiscard]] math::Vec3
gravitational_acceleration(const math::Vec3 &position,
                           double gravitational_parameter);

}