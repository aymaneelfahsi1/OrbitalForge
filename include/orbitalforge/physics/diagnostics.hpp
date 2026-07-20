#pragma once

#include "orbitalforge/math/vec3.hpp"
#include "orbitalforge/physics/system_state.hpp"

namespace orbitalforge::physics {

[[nodiscard]] double kinetic_energy(const SystemState &system) noexcept;

[[nodiscard]] double potential_energy(const SystemState &system,
                                      double gravitational_constant);

[[nodiscard]] double total_energy(const SystemState &system,
                                  double gravitational_constant);

[[nodiscard]] math::Vec3 total_momentum(const SystemState &system) noexcept;

[[nodiscard]] math::Vec3 center_of_mass(const SystemState &system);

} // namespace orbitalforge::physics
