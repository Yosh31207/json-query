// -----------------------------------
// Copyright (c) Yosh31207
// Distributed under the MIT license.
// -----------------------------------

#include <json_query/json_query.hpp>

#include <array>
#include <string_view>
#include <type_traits>

#include <boost/json.hpp>
#include <gtest/gtest.h>

using namespace std::string_view_literals;

#define WRAP_STRING(string_literal) []() constexpr { return std::string_view(string_literal); }

namespace {

template <typename T>
constexpr bool is_const_reference_v = std::is_const_v<std::remove_reference_t<T>>;

TEST(JsonQueryInternal, StrToIndex) {
    using namespace json_query_internal;
    static_assert(str_to_index("0") == 0);
    static_assert(str_to_index("9") == 9);
    static_assert(str_to_index("007") == 7);
    static_assert(str_to_index("123") == 123);
}

TEST(JsonQueryInternal, CountJsonPaths) {
    using namespace json_query_internal;
    static_assert(count_json_paths("foo.bar.baz") == 3);
    static_assert(count_json_paths("foo.bar[1].baz") == 4);
    static_assert(count_json_paths("foo.bar.baz[2]") == 4);
}

TEST(JsonQueryInternal, ParseJsonPaths) {
    using namespace json_query_internal;
    using std::array;

    static_assert(
        parse_json_paths(WRAP_STRING("foo.bar.baz")) ==
        array<KeyOrIndex, 3>{"foo", "bar", "baz"});

    static_assert(
        parse_json_paths(WRAP_STRING("foo.bar[1].baz")) ==
        array<KeyOrIndex, 4>{"foo", "bar", size_t{1}, "baz"});

    static_assert(
        parse_json_paths(WRAP_STRING("foo.bar.baz[2]")) ==
        array<KeyOrIndex, 4>{"foo", "bar", "baz", size_t{2}});

    static_assert(
        parse_json_paths(WRAP_STRING("foo.1st.2[0]")) ==
        array<KeyOrIndex, 4>{"foo", "1st", "2", size_t{0}});

    static_assert(
        get_path_types(array<KeyOrIndex, 4>{"foo", "bar", size_t{0}, "baz"}) ==
        array<PathType, 4>{PathType::ObjetKey, PathType::ObjetKey, PathType::ArrayIndex, PathType::ObjetKey});
}

TEST(JsonQueryInternal, ParseJson) {
    using namespace json_query_internal;
    using std::tuple;

    static_assert(
        parse_json(WRAP_STRING("foo.bar.baz")) ==
        tuple{"foo", "bar", "baz"});

    static_assert(
        parse_json(WRAP_STRING("foo.bar[0].baz")) ==
        tuple{"foo", "bar", 0, "baz"});

    static_assert(
        parse_json(WRAP_STRING("foo.bar.baz[2]")) ==
        tuple{"foo", "bar", "baz", 2});

    static_assert(
        parse_json(WRAP_STRING("foo.bar.1st[1][02][30].values[1][2]")) ==
        tuple{"foo", "bar", "1st", 1, 2, 30, "values", 1, 2});
}

// This shows the usage of this utility
TEST(JsonQuery, QueryJson) {
    auto json = boost::json::parse(R"(
      {
        "foo": {
          "users": [
            {
              "id": 1,
              "name": "Alice"
            },
            {
              "id": 2,
              "name": "Bob"
            },
            {
              "id": 3,
              "name": "Carol",
              "favorites": ["C++", "Rust", "Python"]
            }
          ]
        }
      }
    )");

    // extract as boost::json::value&
    EXPECT_EQ(
        QUERY_JSON(json, foo.users[0].id).as_int64(),
        1);
    EXPECT_EQ(
        QUERY_JSON(json, foo.users[0].name).as_string(),
        "Alice");

    // extract as specified type
    EXPECT_EQ(
        QUERY_JSON(json, foo.users[0].id, int64),
        1);
    EXPECT_EQ(
        QUERY_JSON(json, foo.users[0].name, string),
        "Alice");

    // value can be changed
    QUERY_JSON(json, foo.users[1].name) = "New name";
    EXPECT_EQ(json.at("foo").at("users").at(1).at("name").as_string(), "New name");

    // when passing a const reference, the returned value will also be const reference
    {
        const boost::json::value& const_json = json;
        static_assert(is_const_reference_v<decltype(QUERY_JSON(const_json, foo.users[1].id))>);
        static_assert(is_const_reference_v<decltype(QUERY_JSON(const_json, foo.users[1].id, string))>);
    }

    // throws exception if the key does not exist
    EXPECT_ANY_THROW(QUERY_JSON(json, inexistent.key.name));

    // throws exception if the type of the value does not match
    EXPECT_ANY_THROW(QUERY_JSON(json, foo.users[1].name, int64));

    // test case for when the last element is array type
    EXPECT_EQ(QUERY_JSON(json, foo.users[2].favorites[0]).as_string(), "C++");
    EXPECT_EQ(QUERY_JSON(json, foo.users[2].favorites[1], string), "Rust");

    // text case for key names starting with a number
    auto json2 = boost::json::parse(R"(
        {
          "foo": {
            "1st": {
              "2": [3, 4, 5]
            }
          }
        }
    )");
    // clang-format off
    EXPECT_EQ(QUERY_JSON(json2, foo.1st.2[0]).as_int64(), 3);
    EXPECT_EQ(QUERY_JSON(json2, foo.1st.2[1], int64), 4);
    // clang-format on
}

} // namespace
