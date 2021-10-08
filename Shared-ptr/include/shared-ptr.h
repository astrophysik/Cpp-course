#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

template<typename T>
class shared_ptr;

template<typename T>
class weak_ptr;

namespace shared_ptr_details {
struct base_control_block {
  template<typename T>
  friend class ::shared_ptr;

  template<typename T>
  friend class ::weak_ptr;

  base_control_block() : ref_count(1), weak_count(1) {}

  void inc_ref();

  void dec_ref();

  void inc_weak();

  void dec_weak();

  virtual void delete_obj() = 0;

  virtual ~base_control_block() = default;

private:
  size_t ref_count;
  size_t weak_count;
};

template<typename T, typename D>
struct pointer_control_block : base_control_block {
public:
  pointer_control_block(T *ptr_)
      : base_control_block(), deleter(std::default_delete<T>()), ptr(ptr_) {}

  pointer_control_block(T *ptr_, D &&deleter)
      : base_control_block(), deleter(std::move(deleter)), ptr(ptr_) {}

  void delete_obj() override {
    deleter(ptr);
  }

private:
  [[no_unique_address]] D deleter;
  T *ptr;
};

template<typename T>
struct inplace_control_block : base_control_block {
public:
  template<typename... Args>
  inplace_control_block(Args &&...args) : base_control_block() {
    new (&obj) T(std::forward<Args>(args)...);
  }

  void delete_obj() override {
    reinterpret_cast<T &>(obj).~T();
  }

public:
  typename std::aligned_storage_t<sizeof(T), alignof(T)> obj;
};
}// namespace shared_ptr_details

template<typename T>
class shared_ptr {
  template<typename U>
  friend class weak_ptr;

  template<typename U>
  friend class shared_ptr;

public:
  constexpr shared_ptr() noexcept : control_block(nullptr), ptr(nullptr) {}

  constexpr shared_ptr(std::nullptr_t) noexcept : shared_ptr() {}

  template<typename Tp, typename = std::enable_if_t<std::is_convertible<Tp *, T *>::value>>
  shared_ptr(Tp *ptr_) try : control_block(new shared_ptr_details::pointer_control_block<Tp, std::default_delete<Tp>>(ptr_)), ptr(ptr_) {}
  catch (...) {
    delete ptr_;
  }

  template<typename Tp, typename D, typename = std::enable_if_t<std::is_constructible<Tp *, T *>::value>>
  shared_ptr(Tp *ptr_, D &&deleter = D()) try : control_block(new shared_ptr_details::pointer_control_block<Tp, D>(ptr_, std::forward<D>(deleter))), ptr(ptr_) {}
  catch (...) {
    delete ptr_;
  }

  shared_ptr(const shared_ptr<T> &other) noexcept
      : control_block(other.control_block), ptr(other.ptr) {
    inc_cblock();
  }

  template<typename Tp, typename = std::enable_if_t<std::is_convertible<Tp *, T *>::value>>
  shared_ptr(const shared_ptr<Tp> &other) noexcept
      : control_block(other.control_block), ptr(other.ptr) {
    inc_cblock();
  }

  shared_ptr(shared_ptr &&other) noexcept
      : control_block(other.control_block), ptr(other.ptr) {
    inc_cblock();
    other.reset();
  }

  template<typename U>
  shared_ptr(const shared_ptr<U> &r, T *element) : control_block(r.control_block), ptr(element) {
    inc_cblock();
  }

  shared_ptr &operator=(const shared_ptr &other) noexcept {
    if (this == &other) {
      return *this;
    }
    shared_ptr(other).swap(*this);
    return *this;
  }

  shared_ptr &operator=(shared_ptr &&other) noexcept {
    if (this == &other) {
      return *this;
    }
    shared_ptr(other).swap(*this);
    other.reset();
    return *this;
  }

  operator bool() const noexcept {
    return get() != nullptr;
  }

  operator shared_ptr<T const>() noexcept {
    inc_cblock();
    return shared_ptr<T const>(control_block, ptr);
  }

  T *get() const noexcept {
    return ptr;
  }

  T &operator*() const noexcept {
    return *get();
  }

  T *operator->() const noexcept {
    return get();
  }

  std::size_t use_count() const noexcept {
    if (control_block) {
      return control_block->ref_count;
    } else {
      return 0;
    }
  }

  void reset() noexcept {
    dec_cblock();
    ptr = nullptr;
    control_block = nullptr;
  }

  template<typename Tp, typename D = std::default_delete<Tp>, typename = std::enable_if<std::is_constructible<Tp *, T *>::value>>
  void reset(Tp *new_ptr, D &&deleter = D()) {
    shared_ptr_details::base_control_block * new_control_block;
    try {
      new_control_block = new shared_ptr_details::pointer_control_block<Tp, D>(new_ptr, std::forward<D>(deleter));
    } catch (...) {
      delete new_ptr;
    }
    dec_cblock();
    control_block = new_control_block;
    ptr = new_ptr;
  }

  void swap(shared_ptr &other) noexcept {
    std::swap(ptr, other.ptr);
    std::swap(control_block, other.control_block);
  }

  ~shared_ptr() {
    dec_cblock();
  }

  template<typename U, typename... Args>
  friend shared_ptr<U> make_shared(Args &&...args);

  template<typename U>
  friend bool operator==(const shared_ptr<U> &, std::nullptr_t);

  template<typename U>
  friend bool operator==(std::nullptr_t, const shared_ptr<U> &);

  template<typename U>
  friend bool operator!=(const shared_ptr<U> &, std::nullptr_t);

  template<typename U>
  friend bool operator!=(std::nullptr_t, const shared_ptr<U> &);

private:
  void inc_cblock() const {
    if (control_block) {
      control_block->inc_ref();
    }
  }

  void dec_cblock() const {
    if (control_block) {
      control_block->dec_ref();
    }
  }

  shared_ptr(shared_ptr_details::base_control_block *block, T *ptr) noexcept
      : control_block(block), ptr(ptr) {}

  shared_ptr_details::base_control_block *control_block;
  T *ptr;
};

template<typename T>
class weak_ptr {
public:
  constexpr weak_ptr() noexcept : control_block(nullptr), ptr(nullptr) {}

  weak_ptr(const weak_ptr &other) noexcept : control_block(other.control_block), ptr(other.ptr) {
    inc_cblock();
  }

  weak_ptr(const shared_ptr<T> &other) noexcept
      : control_block(other.control_block), ptr(other.ptr) {
    inc_cblock();
  }

  weak_ptr &operator=(const weak_ptr &other) noexcept {
    if (this == &other) {
      return *this;
    }
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr &operator=(const shared_ptr<T> &other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr &operator=(weak_ptr &&other) noexcept {
    if (this == &other) {
      return *this;
    }
    weak_ptr(other).swap(*this);
    dec_cblock();
    other.control_block = nullptr;
    other.ptr = nullptr;
    return *this;
  }

  shared_ptr<T> lock() const noexcept {
    if (!control_block || control_block->ref_count == 0) {
      return shared_ptr<T>();
    }
    control_block->inc_ref();
    return shared_ptr<T>(control_block, ptr);
  }

  void swap(weak_ptr &other) noexcept {
    std::swap(control_block, other.control_block);
    std::swap(ptr, other.ptr);
  }

  ~weak_ptr() {
    dec_cblock();
  }

private:
  void inc_cblock() const {
    if (control_block) {
      control_block->inc_weak();
    }
  }

  void dec_cblock() const {
    if (control_block) {
      control_block->dec_weak();
    }
  }
  shared_ptr_details::base_control_block *control_block;
  T *ptr;
};

template<typename T, typename... Args>
shared_ptr<T> make_shared(Args &&...args) {
  auto it = new shared_ptr_details::inplace_control_block<T>(std::forward<Args>(args)...);
  return shared_ptr<T>(it, &reinterpret_cast<T &>(it->obj));
}

template<typename T>
bool operator==(std::nullptr_t, const shared_ptr<T> &ptr) {
  return !ptr;
}

template<typename T>
bool operator==(const shared_ptr<T> &ptr, std::nullptr_t) {
  return !ptr;
}

template<typename T>
bool operator!=(const shared_ptr<T> &ptr, std::nullptr_t) {
  return ptr;
}

template<typename T>
bool operator!=(std::nullptr_t, const shared_ptr<T> &ptr) {
  return ptr;
}
