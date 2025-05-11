# JSON_QUERY for Boost.JSON

Single-header library that provides utility for [Boost.JSON](https://github.com/boostorg/json), allowing access to the inner data of JSON value using JavaScript-like syntax.

## Requirements

* C++20
* Boost libraries

This library internally uses Boost libraries (Boost.JSON, Boost.Hana, and Boost.Preprocessor). Please ensure that the Boost root directory (`BOOST_ROOT`) is added to the include directories of your project.

## Usage

This library provides `QUERY_JSON` macro, that allows accessing an inner element of `boost::json::value`.

### Syntax

```C++
// two arguments version
QUERY_JSON(json, path)

// three arguments version
QUERY_JSON(json, path, type)
```

`json`:

Reference to a `boost::json::value`. You can pass both of const and non-const reference types.

`path`:

Path to the inner data of JSON. The path can be written in JavaScript-like syntax as like `foo.bar[0].baz[1][2]`.

`type` (optional):

Underlying type of `boost::json::value`. The type can be any of the followings:

* int64
* uint64
* double
* bool
* string
* array
* object

```C++
boost::json::value json;

// Equivalent to:
//   json.at("foo").at("bar").at(0).at("baz).at(1).at(2);
auto& value = QUERY_JSON(json, foo.bar[0].baz[1][2]);

// Equivalent to:
//   auto& value = json.at("foo").at("bar").at(0).at("baz).as_int64();
auto& value = QUERY_JSON(json, foo.bar[0].baz, int64);

// Equivalent to:
//   auto& value = json.at("foo").at("bar").at(0).at("baz).as_string();
auto& value = QUERY_JSON(json, foo.bar[0].baz, string);

// Key names starting with a number are also okay.
// The following case is equivalent to:
//   auto& value = json.at("foo").at("1st").at("2").at("baz").at(3);
auto& value = QUERY_JSON(json, foo.1st.2.baz[3]);
```

### Exception

These macros may throw exceptions defined in Boost.JSON.

```C++
// This may raise the same exception as the one
// that was thrown when calling
// json.at("foo").at("bar").at(0).at("baz").as_int64().
QUERY_JSON(json, foo.bar[0].baz, int64);
```

## Remarks

Array index must be a number literal. Passing variables is not supported.

```C++
QUERY_JSON(json, foo.bar[0]);   // OK
QUERY_JSON(json, foo.bar[123]); // OK

size_t n = 0;
QUERY_JSON(json, foo.bar[n]);   // Compile error
```

## Example

```C++
#include <cassert>

#include <boost/json.hpp>
#include <json_query/json_query.hpp>

int main() {
    const char* json_text = R"(
      {
        "foo": {
          "users": [
            { "id": 1, "name": "Alice" },
            { "id": 2, "name": "Bob" }
          ]
        }
      }
    )";

    boost::json::value json = boost::json::parse(json_text);

    // Get the inner data
    assert(QUERY_JSON(json, foo.users[0].id, int64) == 1);
    assert(QUERY_JSON(json, foo.users[0].name, string) == "Alice");

    // You can modify the value unless the input is const-reference.
    QUERY_JSON(json, foo.users[0].name) = "New name";

    return 0;
}
```

## Tested With

* Boost: 1.88

* Environment
  * Ubuntu 24.04 (WSL2)
  * GCC: 13.3.0
