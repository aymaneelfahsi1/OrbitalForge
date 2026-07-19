#pragma once

#include <iosfwd>
namespace orbitalforge::math {

class Vec3 {
public:
  constexpr Vec3() = default;

  constexpr Vec3(double x, double y, double z) : x_{x}, y_{y}, z_{z} {}

  [[nodiscard]] constexpr double x() const noexcept { return x_; }

  [[nodiscard]] constexpr double y() const noexcept { return y_; }

  [[nodiscard]] constexpr double z() const noexcept { return z_; }

  [[nodiscard]] constexpr double squared_norm() const noexcept {
    return x_ * x_ + y_ * y_ + z_ * z_;
  }

  [[nodiscard]] double
  norm() const noexcept; // No function definition just a declaration

  Vec3 &operator+=(const Vec3 &other) noexcept;

  [[nodiscard]] constexpr double dot(const Vec3 &other) const noexcept {
    return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
  }

  [[nodiscard]] Vec3 normalized() const;

private:
  double x_{};
  double y_{};
  double z_{};
};

// Because the operators are symmetrical, we will define them outside the class
// (not members, they are called free functions)

[[nodiscard]] inline Vec3 operator+(Vec3 lhs, const Vec3 &rhs) noexcept {

  lhs += rhs;
  return lhs;
}

[[nodiscard]] constexpr Vec3 operator-(const Vec3 &lhs,
                                       const Vec3 &rhs) noexcept {
  return Vec3{lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z()};
}

[[nodiscard]] constexpr Vec3 operator*(const Vec3 &vector,
                                       double scalar) noexcept {
  return Vec3{vector.x() * scalar, vector.y() * scalar, vector.z() * scalar};
}

[[nodiscard]] constexpr Vec3 operator*(double scalar,
                                       const Vec3 &vector) noexcept {
  return Vec3{scalar * vector.x(), scalar * vector.y(), scalar * vector.z()};
}

[[nodiscard]] constexpr bool operator==(const Vec3 &lhs,
                                        const Vec3 &rhs) noexcept {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y() && lhs.z() == rhs.z();
}

[[nodiscard]] constexpr Vec3 operator/(const Vec3 &vector,
                                       double scalar) noexcept {
  return Vec3{vector.x() / scalar, vector.y() / scalar, vector.z() / scalar};
}

std::ostream &operator<<(std::ostream &output, const Vec3 &vector);

} // namespace orbitalforge::math