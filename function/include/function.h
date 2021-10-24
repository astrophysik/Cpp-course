#pragma once

#include <type_traits>

using bad_function_call = std::bad_function_call;

namespace function_details {

using func_storage = std::aligned_storage_t<sizeof(void *), alignof(void *)>; ///std::byte[8]

template <typename T>
constexpr T * get_ptr(func_storage * storage) {
  return static_cast<T *>(*reinterpret_cast<void**>(storage));
}

template <typename T>
constexpr const T * get_ptr(const func_storage * storage) {
  return static_cast<const T *>(*reinterpret_cast<void*const*>(storage));
}

template <typename T>
constexpr bool object_is_small() {
  return sizeof(T) < sizeof(void*) && alignof(T) < alignof(void*) &&
         std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>;
}

template <typename T>
constexpr void deleter(func_storage * func) {
  if constexpr (object_is_small<T>()) {
    reinterpret_cast<T&>(*func).~T();
  } else {
    delete get_ptr<T>(func);
  }
}

template <typename T>
constexpr void copier(func_storage * dst, const func_storage * src) {
  if constexpr (object_is_small<T>()) {
    new (dst) T(reinterpret_cast<const T&>(*src));
  } else {
    *reinterpret_cast<void**>(dst) = new T(*get_ptr<T>(src));
  }
}

template <typename T>
constexpr void mover(func_storage * dst, func_storage * src) {
  if constexpr (object_is_small<T>()) {
    reinterpret_cast<T&>(*dst) = (reinterpret_cast<T&>(*src));
  } else {
    *dst = *src;
  }
}

template <typename T, typename R, typename... Args>
constexpr R invoker(const func_storage * func, Args... args) {
  if constexpr (object_is_small<T>()) {
    return (reinterpret_cast<T const&>(*func))(std::forward<Args>(args)...);
  } else {
    return (*get_ptr<T>(func))(std::forward<Args>(args)...);
  }
}

template <typename T>
constexpr void* getter(const func_storage * func) {
  if constexpr (object_is_small<T>()) {
    return reinterpret_cast<T*>(const_cast<func_storage *>(func));
  } else {
    return const_cast<T *>(get_ptr<T>(func));
  }
}

template <typename R, typename... Args>
struct operations {
  using deleter_t = void (*)(func_storage*);
  using copier_t = void (*)(func_storage*, const func_storage*);
  using mover_t = void (*)(func_storage*, func_storage*);
  using invoker_t = R (*)(const func_storage*, Args...);
  using getter_t = void* (*)(const func_storage*);


  constexpr operations(deleter_t del, copier_t cop, mover_t mov, invoker_t inv,
                       getter_t get)
      : deleter(del), copier(cop), mover(mov), invoker(inv), getter(get) {}

  deleter_t deleter;
  copier_t copier;
  mover_t mover;
  invoker_t invoker;
  getter_t getter;
};

template <typename T, typename R, typename... Args>
static const operations<R, Args...>* get_operations() {
  constexpr static operations<R, Args...> operations(
      deleter<T>, copier<T>, mover<T>, invoker<T>, getter<T>);
  return &operations;
}
}

template<typename F>
struct function;

template<typename R, typename... Args>
struct function<R(Args...)> {

  function() noexcept = default;

    function(function const &other) {
        if (other.ops) {
            other.ops->copier(&obj, &other.obj);
            ops = other.ops;
        } else {
            ops = nullptr;
        }
    }

    function(function &&other) noexcept {
        if (other.ops) {
            ops = other.ops;
            ops->mover(&obj, &other.obj);
        }
      other.ops = nullptr;
    }

  template<typename T>
  function(T val) : ops(function_details::get_operations<T, R, Args...>()) {
    if (function_details::object_is_small<T>()) {
      new (&obj) T(val);
    } else {
      *reinterpret_cast<void **>(&obj) = new T(std::move(val));
    }
  }

  function &operator=(function const &rhs) {
    if (this == &rhs) {
      return *this;
    }
    function(rhs).swap(*this);
    return *this;
  }

  function &operator=(function &&rhs) noexcept {
    if (this == &rhs) {
      return *this;
    }
    swap(rhs);
    function().swap(rhs);
    return *this;
  }

  ~function() {
    if (ops) {
      ops->deleter(&obj);
    }
  }

  explicit operator bool() const noexcept {
    return ops != nullptr;
  }

  R operator()(Args... args) const {
    if (!ops) {
      throw std::bad_function_call{};
    }
    return ops->invoker(&obj, std::forward<Args>(args)...);
  }

  template<typename T>
  T *target() noexcept {
    if (ops != function_details::get_operations<T, R, Args...>()) {
      return nullptr;
    }
    return reinterpret_cast<T *>(ops->getter(&obj));
  }

  template<typename T>
  T const *target() const noexcept {
    if (ops != function_details::get_operations<T, R, Args...>()) {
      return nullptr;
    }
    return reinterpret_cast<T const *>(ops->getter(&obj));
  }

  void swap(function &other) noexcept {
    std::swap(obj, other.obj);
    std::swap(ops, other.ops);
  }

private:
  function_details::func_storage obj;
  const function_details::operations<R, Args...> * ops{nullptr};
};
