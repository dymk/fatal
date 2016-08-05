/*
 *  Copyright (c) 2016, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 */

#include <fatal/container/static_array.h>

#include <fatal/type/debug.h>
#include <fatal/type/foreach.h>
#include <fatal/type/list.h>
#include <fatal/type/sequence.h>
#include <fatal/type/slice.h>
#include <fatal/type/type.h>
#include <fatal/type/zip.h>

#include <fatal/test/driver.h>

#include <type_traits>

namespace fatal {

struct abc { int x; int y; int z; };

struct str {
  FATAL_STR(hello, "hello");
  FATAL_STR(world, "world");
  FATAL_STR(test, "test");
};

struct check_abc_visitor {
  template <typename T, std::size_t Index, typename U>
  void operator ()(indexed<T, Index>, U const &array) const {
    FATAL_ASSERT_LT(Index, array.size());
    FATAL_EXPECT_EQ(first<T>::value, array[Index].x);
    FATAL_EXPECT_EQ(second<T>::value, array[Index].y);
    FATAL_EXPECT_EQ(third<T>::value, array[Index].z);
  }
};

struct abc_factory {
  template <typename T>
  static constexpr abc get() {
    return abc{
      first<T>::value,
      second<T>::value,
      third<T>::value
    };
  }
};

template <typename Expected, typename... T>
void check_abc_array() {
  using array = static_array<Expected, abc_factory, T...>;
  static_assert(size<Expected>::value == array::get.size(), "size mismatch");
  foreach<Expected>(check_abc_visitor(), array::get);
}

template <typename x, typename y, typename z>
void check_abc() {
  using expected = fatal::zip<list, list, x, y, z>;
  check_abc_array<expected>();
  check_abc_array<expected, abc>();
}

FATAL_TEST(static_array, struct) {
  check_abc<int_list<0>, int_list<0>, int_list<0>>();
  check_abc<int_list<0>, int_list<1>, int_list<2>>();
  check_abc<int_list<99>, int_list<56>, int_list<43>>();

  check_abc<int_list<0, 0>, int_list<0, 1>, int_list<0, 2>>();

  check_abc<int_list<0, 0, 99>, int_list<0, 1, 56>, int_list<0, 2, 43>>();
  check_abc<int_list<0, 3, 6>, int_list<1, 4, 7>, int_list<2, 5, 8>>();

  check_abc<
    int_list<0, 3, 6, 9>,
    int_list<1, 4, 7, 10>,
    int_list<2, 8, 5, 11>
  >();
  check_abc<
    int_list<99, 3, 0, 5>,
    int_list<15, 8, 46, 1>,
    int_list<62, 12, 85, 7>
  >();
}

template <typename T, typename... Values>
struct check_sequence_list {
  template <typename... U>
  static void impl() {
    using expected_type = std::array<T, sizeof...(Values)>;
    using actual = static_z_array<list<Values...>, U...>;
    expected_type const expected{{ z_data<Values>()...  }};

    FATAL_EXPECT_SAME<
      T,
      value_type_of<typename std::decay<decltype(actual::get)>::type>
    >();
    FATAL_EXPECT_SAME<
      expected_type,
      typename std::decay<decltype(actual::get)>::type
    >();
    FATAL_EXPECT_EQ(sizeof...(Values), actual::get.size());
    FATAL_EXPECT_EQ(expected, actual::get);
  };

  static void check() {
    impl<>();
    impl<T>();
  }
};

template <typename T>
struct check_sequence_list<T> {
  static void check() {
    using actual = static_z_array<list<>, T>;

    FATAL_EXPECT_SAME<
      T,
      value_type_of<typename std::decay<decltype(actual::get)>::type>
    >();
    FATAL_EXPECT_SAME<
      std::array<T, 0>,
      typename std::decay<decltype(actual::get)>::type
    >();
    FATAL_EXPECT_EQ(0, actual::get.size());
  }
};

FATAL_TEST(static_array, sequence list) {
  check_sequence_list<char const *>::check();
  check_sequence_list<char const *, str::hello>::check();
  check_sequence_list<char const *, str::hello, str::world>::check();
  check_sequence_list<
    char const *, str::hello, str::world, str::test
  >::check();
}

} // namespace fatal {
