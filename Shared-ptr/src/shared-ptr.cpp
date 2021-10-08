#include "shared-ptr.h"

void shared_ptr_details::base_control_block::inc_ref() {
  ++ref_count;
  inc_weak();
}

void shared_ptr_details::base_control_block::dec_ref() {
  --ref_count;
  if (ref_count == 0) {
    delete_obj();
  }
  dec_weak();
}

void shared_ptr_details::base_control_block::inc_weak() {
  ++weak_count;
}

void shared_ptr_details::base_control_block::dec_weak() {
  --weak_count;
  if (weak_count == 0) {
    delete this;
  }
}
