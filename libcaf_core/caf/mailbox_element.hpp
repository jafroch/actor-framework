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

#ifndef CAF_MAILBOX_ELEMENT_HPP
#define CAF_MAILBOX_ELEMENT_HPP

#include <cstdint>

#include "caf/fwd.hpp" // mailbox_element_uptr
#include "caf/extend.hpp"
#include "caf/message.hpp"
#include "caf/actor_addr.hpp"
#include "caf/message_id.hpp"
#include "caf/ref_counted.hpp"

#include "caf/mixin/memory_cached.hpp"

#include "caf/detail/tuple_vals.hpp"
#include "caf/detail/message_data.hpp"
#include "caf/detail/implicit_conversions.hpp"
#include "caf/detail/mailbox_element_disposer.hpp"

// needs access to constructor + destructor to initialize m_dummy_node
namespace caf {

class mailbox_element {
 public:
  friend class detail::mailbox_element_disposer;

  mailbox_element* next; // intrusive next pointer
  bool marked;           // denotes if this node is currently processed
  actor_addr sender;
  message_id mid;

  mailbox_element();

  virtual message& msg() = 0;

  mailbox_element(mailbox_element&&) = delete;
  mailbox_element(const mailbox_element&) = delete;
  mailbox_element& operator=(mailbox_element&&) = delete;
  mailbox_element& operator=(const mailbox_element&) = delete;

  template <class T, class... Ts>
  static typename std::enable_if<
    !std::is_same<typename detail::strip_and_convert<T>::type, message>::value,
    mailbox_element_uptr
  >::type
  create(actor_addr sender, message_id id, T&& v, Ts&&... vs) {
    using base = detail::tuple_vals<typename detail::strip_and_convert<T>::type,
                                    typename detail::strip_and_convert<Ts>::type...>;
    class impl : public mailbox_element, public base {
     public:
      message m_self;
      impl(actor_addr& sender, message_id id, T&& v, Ts&&... vs)
          : mailbox_element(std::move(sender), id),
            base(std::forward<T>(v), std::forward<Ts>(vs)...) {
        // holds an intrusive pointer to itself,
        // which is released in dispose_mailbox_element()
        message tmp{this};
        m_self.swap(tmp);
      }
      void dispose_mailbox_element() override {
        // deletes instance if no other reference exists
        m_self.reset();
      }
      message& msg() override {
        return m_self;
      }
    };
    return mailbox_element_uptr{new impl(sender, id, std::forward<T>(v),
                                         std::forward<Ts>(vs)...)};
  }

  static mailbox_element_uptr create(actor_addr sender, message_id id,
                                     message content);

 protected:
  mailbox_element(actor_addr sender, message_id id);
  virtual ~mailbox_element();
  virtual void dispose_mailbox_element() = 0;
};

} // namespace caf

#endif // CAF_MAILBOX_ELEMENT_HPP
