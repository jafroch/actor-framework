/******************************************************************************
 *                       ____    _    _____                                   *
 *                      / ___|  / \  |  ___|    C++                           *
 *                     | |     / _ \ | |_       Actor                         *
 *                     | |___ / ___ \|  _|      Framework                     *
 *                      \____/_/   \_|_|                                      *
 *                                                                            *
 * Copyright (C) 2011 - 2014                                                  *
 * Dominik Charousset <dominik.charousset (at) haw-hamburg.de>                *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 *                                                                            *
 * If you did not receive a copy of the license files, see                    *
 * http://opensource.org/licenses/BSD-3-Clause and                            *
 * http://www.boost.org/LICENSE_1_0.txt.                                      *
 ******************************************************************************/

#ifndef CAF_THREADED_HPP
#define CAF_THREADED_HPP

#include <mutex>
#include <chrono>
#include <condition_variable>

#include "caf/exit_reason.hpp"
#include "caf/mailbox_element.hpp"

#include "caf/detail/sync_request_bouncer.hpp"
#include "caf/detail/single_reader_queue.hpp"

#include "caf/policy/invoke_policy.hpp"

namespace caf {
namespace policy {

class nestable_invoke : public invoke_policy<nestable_invoke> {
 public:
  bool hm_should_skip(const mailbox_element_uptr& node) {
    return node->marked;
  }

  template <class Actor>
  mailbox_element_uptr hm_begin(Actor* self, mailbox_element_uptr& msg) {
    msg->marked = true;
    mailbox_element_uptr previous;
    self->current_msg().swap(previous);
    self->current_msg().swap(msg);
    self->push_timeout();
    return previous;
  }

  template <class Actor>
  void hm_cleanup(Actor* self, mailbox_element_uptr& previous,
                  mailbox_element_uptr& msg) {
    self->current_msg()->marked = false;
    self->current_msg().swap(msg);
    self->current_msg().swap(previous);
    self->pop_timeout();
  }

  template <class Actor>
  void hm_revert(Actor* self, mailbox_element_uptr& previous,
                 mailbox_element_uptr& msg) {
    // same operation for blocking, i.e., nestable, invoke
    hm_cleanup(self, previous, msg);
  }
};

} // namespace policy
} // namespace caf

#endif // CAF_THREADED_HPP
