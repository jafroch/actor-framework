#include "framework/unit.hpp"

CAF_SUITE("example")

CAF_TEST("foo")
{
  CAF_CHECK(true);
  CAF_CHECK(false);
}

CAF_TEST("bar")
{
  auto i = 42;
  i *= 2;
  CAF_REQUIRE(i == 84);
}
