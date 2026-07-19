#pragma once

#include <string>

#include <orbitalforge/math/vec3.hpp>
#include <orbitalforge/physics/state.hpp>

namespace orbitalforge::physics {
using math::Vec3;
using std::string;

struct Body {
  Body(string name, double mass, Vec3 position, Vec3 velocity);

  [[nodiscard]] State state() const noexcept;

  void set_state(const State &new_state) noexcept;

  string name;
  double mass;
  Vec3 position;
  Vec3 velocity;
};

} // namespace orbitalforge::physics