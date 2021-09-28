#include "intrusive_list.h"

namespace intrusive {

base_list_element::base_list_element(base_list_element&& other) : next(other.next), prev(other.prev) {
  other.next = nullptr;
  other.prev = nullptr;
  if (next) {
    next->prev = this;
  }
  if (prev) {
    prev->next = this;
  }
}

bool base_list_element::is_linked() const {
  return prev != nullptr;
}

void base_list_element::unlink() {
  if (is_linked()) {
    prev->next = next;
    next->prev = prev;
    next = nullptr;
    prev = nullptr;
  }
}

void base_list_element::tie() {
  next = this;
  prev = this;
}

void base_list_element::link_before(base_list_element* element) {
  next = element;
  prev = element->prev;
  element->prev->next = this;
  element->prev = this;
}

void base_list_element::link_with_next(base_list_element* element) {
  next = element;
  element->prev = this;
}

base_list_element::~base_list_element() {
  unlink();
}
}