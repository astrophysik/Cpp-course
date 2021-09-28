#pragma once

#include <iterator>
#include <type_traits>

namespace intrusive {
struct default_tag;

template <typename T, typename Tag>
struct list;

struct base_list_element {
  template <typename T, typename Tag>
  friend struct list;

  base_list_element() = default;

  base_list_element(base_list_element&& other);

  base_list_element(const base_list_element&) = delete;

  ~base_list_element();

  void unlink();

private:
  base_list_element* next{nullptr};
  base_list_element* prev{nullptr};

  bool is_linked() const;

  void tie();

  void link_before(base_list_element* element);

  void link_with_next(base_list_element* element);
};


template<typename Tag = default_tag>
struct list_element : base_list_element {
  list_element() = default;

  list_element(const list_element&) = delete;

  list_element(list_element && other) : base_list_element(other) {}
};


template<typename T, typename Tag = default_tag>
struct list {
private:
  list_element<Tag> fake_node;

  template<typename U>
  struct base_iterator {
    friend class list<T, Tag>;
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = U;
    using difference_type = std::ptrdiff_t;
    using pointer = U *;
    using reference = U &;
  public:

    operator base_iterator<U const>() noexcept {
      return base_iterator<U const>(ptr);
    }

    base_iterator() = default;

    base_iterator(const base_iterator &source) : ptr(source.ptr) {}

    base_iterator &operator++() {
      ptr = ptr->next;
      return *this;
    }

    base_iterator operator++(int) {
      base_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    reference operator*() const {
      return static_cast<reference>(* static_cast<List_Node *>(ptr));
    }

    pointer operator->() const {
      return static_cast<pointer>(static_cast<List_Node *>(ptr));
    }

    base_iterator &operator--() {
      ptr = ptr->prev;
      return *this;
    }

    base_iterator operator--(int) {
      base_iterator tmp = *this;
      --(*this);
      return tmp;
    }

    friend bool operator!=(const base_iterator<U> &left, const base_iterator<U> &right) {
      return left.ptr != right.ptr;
    }

    friend bool operator==(const base_iterator<U> &left, const base_iterator<U> &right) {
      return left.ptr == right.ptr;
    }

  private:
    using Base_Node = std::conditional_t<std::is_const_v<value_type>, base_list_element const, base_list_element>;

    using List_Node = std::conditional_t<std::is_const_v<value_type>, list_element<Tag> const, list_element<Tag>>;

    base_iterator(Base_Node *source) : ptr(source) {}

    Base_Node *ptr{nullptr};
  };

  void move_from(list && source) noexcept {
    if (!source.empty()) {
      auto first = source.fake_node.next;
      source.fake_node.unlink();
      fake_node.link_before(first);
      source.fake_node.tie();
    } else {
      fake_node.tie();
    }
  }

public:
  using iterator = base_iterator<T>;
  using const_iterator = base_iterator<T const>;

  static_assert(std::is_convertible_v<T &, list_element<Tag> &>,
                "value type is not convertible to list_element");


  list() noexcept {
    fake_node.tie();
  }

  list(list const &) = delete;
  list(list &&source) noexcept {
    move_from(std::move(source));
  }

  list &operator=(const list &) = delete;
  list &operator=(list &&source) noexcept {
    clear();
    move_from(std::move(source));
    return *this;
  }

  ~list() {
    clear();
  }

  iterator begin() noexcept {
    iterator beg(fake_node.next);
    return beg;
  }

  const_iterator begin() const noexcept {
    const_iterator beg(fake_node.next);
    return beg;
  }

  iterator end() noexcept {
    iterator it(&fake_node);
    return it;
  }

  const_iterator end() const noexcept {
    const_iterator it(&fake_node);
    return it;
  }

  iterator insert(const_iterator pos, T &element) noexcept {
    iterator added(static_cast<list_element<Tag> *>(&element));
    iterator tmp(const_cast<base_list_element*>(pos.ptr));
    if (added.ptr->is_linked()) {
      added.ptr->unlink();
    }
    added.ptr->link_before(tmp.ptr);
    return added;
  }

  iterator erase(const_iterator pos) noexcept {
    iterator next(const_cast<base_list_element *>(pos.ptr));
    iterator tmp = next++;
    tmp.ptr->unlink();
    return next;
  }

  void push_back(T &element) noexcept {
    insert(end(), element);
  }

  void pop_back() noexcept {
    erase(--end());
  }

  T &back() noexcept {
    return *(--end());
  }

  T const &back() const noexcept {
    return *(--end());
  }

  void push_front(T &element) noexcept {
    insert(begin(), element);
  }

  void pop_front() noexcept {
    erase(begin());
  }

  T &front() noexcept {
    return *begin();
  }

  T const &front() const noexcept {
    return *begin();
  }

  bool empty() const noexcept {
    return begin() == end();
  }

  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  void splice(const_iterator pos, list &, const_iterator first, const_iterator last) noexcept {
    if (first != last) {
      iterator current(const_cast<base_list_element *>(pos.ptr));
      iterator left(const_cast<base_list_element *>(first.ptr));
      iterator right(const_cast<base_list_element *>(last.ptr));
      auto penultimate = right.ptr->prev;
      left.ptr->prev->next = right.ptr;
      right.ptr->prev = left.ptr->prev;
      current.ptr->prev->link_with_next(left.ptr);
      penultimate->link_with_next(current.ptr);
    }
  }
};
}// namespace intrusive