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

#include "caf/mailbox_element.hpp"

#include "caf/detail/default_mailbox_element.hpp"

namespace caf {

class default_mailbox_element : public mailbox_element {
 public:
  default_mailbox_element(actor_addr& sender, message_id id,
                          message& content)
      : mailbox_element(std::move(sender), id),
        m_msg(std::move(content)) {
    // nop
  }
  message& msg() override {
    return m_msg;
  }
 private:
  message m_msg;
};

mailbox_element::mailbox_element() : next(nullptr), marked(false) {
  // nop
}

mailbox_element::mailbox_element(actor_addr sender, message_id id)
    : next(nullptr),
      marked(false),
      sender(std::move(sender)),
      mid(id) {
  // nop
}

mailbox_element::~mailbox_element() {
  // nop
}

mailbox_element_uptr mailbox_element::create(actor_addr sender,
                                                       message_id id,
                                                       message content) {
  return mailbox_element_uptr{new detail::default_mailbox_element(
    std::move(sender), id, std::move(content))};
}

} // namespace caf
