#include "framework/unit.hpp"

SUITE("example")

TEST("foo")
{
  CHECK(true);
  CHECK(false);
}

TEST("bar")
{
  auto i = 42;
  i *= 2;
  REQUIRE(i == 84);
}
