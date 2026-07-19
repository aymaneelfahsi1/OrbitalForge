#pragma once

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/state.hpp"

namespace orbitalforge::physics {

[[nodiscard]] State explicit_euler_step(const State &state,
                                        const math::Vec3 &acceleration,
                                        double time_step);

[[nodiscard]] State semi_implicit_euler_step(const State &state,
                                             const math::Vec3 &acceleration,
                                             double time_step);

} // namespace orbitalforge::physics