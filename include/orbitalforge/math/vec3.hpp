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

  [[nodiscard]] constexpr Vec3 cross(const Vec3 &other) const noexcept {
    return Vec3{y_ * other.z_ - z_ * other.y_, z_ * other.x_ - x_ * other.z_,
                x_ * other.y_ - y_ * other.x_};
  }

  [[nodiscard]] double
  norm() const noexcept; // No function definition just a declaration

  Vec3 &operator+=(const Vec3 &other) noexcept;
  Vec3 &operator-=(const Vec3 &other) noexcept;
  Vec3 &operator*=(double scalar) noexcept;
  Vec3 &operator/=(double scalar);

  [[nodiscard]] constexpr double dot(const Vec3 &other) const noexcept {
    return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
  }

  [[nodiscard]] Vec3 normalized() const;

private:
  double x_{};
  double y_{};
  double z_{};
};

// Non-member arithmetic operators reuse the compound assignment operators.

[[nodiscard]] inline Vec3 operator+(Vec3 lhs, const Vec3 &rhs) noexcept {

  lhs += rhs;
  return lhs;
}

[[nodiscard]] inline Vec3 operator-(Vec3 lhs, const Vec3 &rhs) noexcept {
  lhs -= rhs;
  return lhs;
}

[[nodiscard]] inline Vec3 operator*(Vec3 vector, double scalar) noexcept {
  vector *= scalar;
  return vector;
}

[[nodiscard]] inline Vec3 operator*(double scalar, Vec3 vector) noexcept {
  vector *= scalar;
  return vector;
}

[[nodiscard]] inline Vec3 operator/(Vec3 vector, double scalar) {

  vector /= scalar;
  return vector;
}

[[nodiscard]] constexpr bool operator==(const Vec3 &lhs,
                                        const Vec3 &rhs) noexcept {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y() && lhs.z() == rhs.z();
}

std::ostream &operator<<(std::ostream &output, const Vec3 &vector);

} // namespace orbitalforge::math