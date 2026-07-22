#pragma once

#include <vector>

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/body.hpp"
#include "orbitalforge/physics/system_state.hpp"

namespace orbitalforge::physics {

using math::Vec3;

[[nodiscard]] Vec3 gravitational_acceleration(const Vec3 &position,
                                              double gravitational_parameter,
                                              double softening = 0.0);

[[nodiscard]] Vec3 gravitational_acceleration(const Body &target,
                                              const Body &source,
                                              double gravitational_constant,
                                              double softening = 0.0);

[[nodiscard]] std::vector<Vec3>
gravitational_accelerations(const SystemState &system,
                            double gravitational_constant,
                            double softening = 0.0);

} // namespace orbitalforge::physics