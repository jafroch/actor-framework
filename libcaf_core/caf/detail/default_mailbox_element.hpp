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

#ifndef CAF_DEFAULT_MAILBOX_ELEMENT_HPP
#define CAF_DEFAULT_MAILBOX_ELEMENT_HPP

#include "caf/mailbox_element.hpp"

namespace caf {
namespace detail {

class default_mailbox_element : public mailbox_element {
 public:
  default_mailbox_element() = default;
  default_mailbox_element(actor_addr&& sender, message_id id,
                          message&& content);
  message& msg() override;
  void dispose_mailbox_element() override;

 private:
  message m_msg;
};

} // namespace detail
} // namespace caf

#endif // CAF_DEFAULT_MAILBOX_ELEMENT_HPP
