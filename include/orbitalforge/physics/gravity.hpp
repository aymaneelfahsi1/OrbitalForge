#pragma once

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"

namespace orbitalforge::physics {

using math::Vec3;
using physics::Body;

[[nodiscard]] Vec3 gravitational_acceleration(const math::Vec3 &position,
                                              double gravitational_parameter);

[[nodiscard]] Vec3 gravitational_acceleration(const Body &target,
                                              const Body &source,
                                              double gravitational_constant);
}; // namespace orbitalforge::physics