#include <iostream>

#include "ping_pong.hpp"
#include "framework/unit.hpp"

#include "caf/all.hpp"
#include "caf/detail/logging.hpp"

using std::cout;
using std::endl;
using namespace caf;
using namespace caf;

namespace {

size_t s_pongs = 0;

behavior ping_behavior(local_actor* self, size_t num_pings) {
  return (on(atom("pong"), arg_match) >> [=](int value)->message {
        if (!self->last_sender()) {
          MESSAGE("last_sender() invalid!");
        }
        MESSAGE("received {'pong', " << value << "}");
        // cout << to_string(self->last_dequeued()) << endl;
        if (++s_pongs >= num_pings) {
          MESSAGE("reached maximum, send {'EXIT', user_defined} "
                 << "to last sender and quit with normal reason");
          self->send_exit(self->last_sender(),
                  exit_reason::user_shutdown);
          self->quit();
        }
        return make_message(atom("ping"), value);
      },
      others() >> [=] {
    CAF_LOGF_ERROR("unexpected message; "
            << to_string(self->last_dequeued()));
    self->quit(exit_reason::user_shutdown);
  });
}

behavior pong_behavior(local_actor* self) {
  return (on(atom("ping"), arg_match) >> [](int value)->message {
        MESSAGE("received {'ping', " << value << "}");
        return make_message(atom("pong"), value + 1);
      },
      others() >> [=] {
    CAF_LOGF_ERROR("unexpected message; "
            << to_string(self->last_dequeued()));
    self->quit(exit_reason::user_shutdown);
  });
}

} // namespace <anonymous>

size_t pongs() { return s_pongs; }

void ping(blocking_actor* self, size_t num_pings) {
  CAF_LOGF_TRACE("num_pings = " << num_pings);
  s_pongs = 0;
  self->receive_loop(ping_behavior(self, num_pings));
}

void event_based_ping(event_based_actor* self, size_t num_pings) {
  CAF_LOGF_TRACE("num_pings = " << num_pings);
  s_pongs = 0;
  self->become(ping_behavior(self, num_pings));
}

void pong(blocking_actor* self, actor ping_actor) {
  CAF_LOGF_TRACE("ping_actor = " << to_string(ping_actor));
  self->send(ping_actor, atom("pong"), 0); // kickoff
  self->receive_loop(pong_behavior(self));
}

void event_based_pong(event_based_actor* self, actor ping_actor) {
  CAF_LOGF_TRACE("ping_actor = " << to_string(ping_actor));
  // FIXME: REQUIRE() can only be called inside TEST().
  //REQUIRE(ping_actor != invalid_actor);
  self->send(ping_actor, atom("pong"), 0); // kickoff
  self->become(pong_behavior(self));
}

SUITE("spawn")

TEST("ping-pong") {
  using namespace caf;
  scoped_actor self;
  self->trap_exit(true);
  auto ping_actor = self->spawn<monitored+blocking_api>(ping, 10);
  auto pong_actor = self->spawn<monitored+blocking_api>(pong, ping_actor);
  self->link_to(pong_actor);
  int i = 0;
  int flags = 0;
  self->delayed_send(self, std::chrono::seconds(1), atom("FooBar"));
  // wait for DOWN and EXIT messages of pong
  self->receive_for(i, 4) (
    [&](const exit_msg& em) {
      CHECK(em.source == pong_actor);
      CHECK(em.reason == exit_reason::user_shutdown);
      flags |= 0x01;
    },
    [&](const down_msg& dm) {
      if (dm.source == pong_actor) {
        flags |= 0x02;
        CHECK(dm.reason == exit_reason::user_shutdown);
      }
      else if (dm.source == ping_actor) {
        flags |= 0x04;
        CHECK(dm.reason == exit_reason::normal);
      }
    },
    [&](const atom_value& val) {
      CHECK(val == atom("FooBar"));
      flags |= 0x08;
    },
    others() >> [&]() {
      //CAF_FAILURE("unexpected message: " << to_string(self->last_dequeued()));
    },
    after(std::chrono::seconds(5)) >> [&]() {
      //CAF_FAILURE("timeout in file " << __FILE__ << " in line " << __LINE__);
    }
  );
  // wait for termination of all spawned actors
  self->await_all_other_actors_done();
  CHECK(flags == 0x0F);
  CHECK(pongs() == 10);
}
