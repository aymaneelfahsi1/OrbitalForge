#include "orbitalforge/math/vec3.hpp"
#include <cmath>
#include <ostream>
#include <stdexcept>

namespace orbitalforge::math {

double Vec3::norm() const noexcept { return std::sqrt(squared_norm()); }

Vec3 &Vec3::operator+=(const Vec3 &other) noexcept {
  x_ += other.x_;
  y_ += other.y_;
  z_ += other.z_;

  return *this;
}

Vec3 Vec3::normalized() const {
  const double length = norm();

  if (length == 0.0) {
    throw std::domain_error{"Cannot normalize a zero vector"};
  }

  return *this / length;
}

std::ostream &operator<<(std::ostream &output, const Vec3 &vector) {
  output << '(' << vector.x() << ", " << vector.y() << ", " << vector.z()
         << ')';

  return output;
}

} // namespace orbitalforge::math
