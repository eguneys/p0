#pragma once

namespace pzero {

// Very poor-man implementation of std::optional. It literally cannot do
// anything, but it's enough for our use case.
template <class T>
class optional {
 public:
  operator bool() const { return has_value_; }
  constexpr const T& operator*() const& { return value_; }
  constexpr const T* operator->() const& { return &value_; }
  optional<T>& operator=(const T& value) {
    value_ = value;
    has_value_ = true;
    return *this;
  }
  void reset() { has_value_ = false; }
  T value_or(const T& def) const { return has_value_ ? value_ : def; }

 private:
  T value_;
  bool has_value_ = false;
};

}  // namespace pzero
