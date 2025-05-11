// -----------------------------------
// Copyright (c) Yosh31207
// Distributed under the MIT license.
// -----------------------------------

#ifndef JSON_QUERY_MACRO_HPP_
#define JSON_QUERY_MACRO_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>

#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>
#include <boost/json.hpp>
#include <boost/preprocessor.hpp>

namespace json_query_internal {

using KeyOrIndex = std::variant<std::string_view, size_t>;

constexpr size_t to_digit(char c) {
    if (c < '0' || '9' < c) {
        throw std::logic_error("Invalid integer literal");
    }
    return static_cast<size_t>(c - '0');
};

constexpr size_t str_to_index_impl(std::string_view str, size_t accum) {
    return str.empty() ? accum
                       : str_to_index_impl(str.substr(1), accum * 10 + to_digit(str.front()));
}

constexpr size_t str_to_index(std::string_view str) {
    return str_to_index_impl(str, 0);
}

constexpr size_t count_json_paths(std::string_view text) {
    return 1 + std::ranges::count_if(
                   text, [](char c) { return c == '.' || c == '['; });
}

template <typename WrappedStringLiteral>
constexpr auto parse_json_paths(WrappedStringLiteral fn) {
    constexpr std::string_view text = fn();
    constexpr size_t size = count_json_paths(text);

    auto result = std::array<KeyOrIndex, size>{};
    size_t count = 0;
    size_t start = 0;
    bool is_array = false;
    for (size_t i = 0; i < text.size(); ++i) {
        switch (text[i]) {
        case '.':
            if (is_array) {
                throw std::logic_error("Invalid syntax");
            }
            if (start < i) {
                result[count++] = text.substr(start, i - start);
            }
            start = i + 1;
            break;
        case '[':
            if (is_array) {
                throw std::logic_error("Invalid syntax");
            }
            is_array = true;
            if (start < i) {
                result[count++] = text.substr(start, i - start);
            }
            start = i + 1;
            break;
        case ']':
            if (!is_array) {
                throw std::logic_error("Invalid syntax");
            }
            is_array = false;
            result[count++] = str_to_index(text.substr(start, i - start));
            start = i + 1;
            break;
        default:
            break;
        }
    }

    if (start < text.size()) {
        result[count++] = text.substr(start);
    }

    if (count != result.size()) {
        throw std::logic_error("Invalid syntax");
    }

    return result;
}

enum class PathType {
    ObjetKey,
    ArrayIndex
};

template <size_t N>
constexpr std::array<PathType, N> get_path_types(std::array<KeyOrIndex, N> paths) {
    auto result = std::array<PathType, N>{};
    for (size_t i = 0; i < N; ++i) {
        auto path_type = std::holds_alternative<size_t>(paths[i]) ? PathType::ArrayIndex : PathType::ObjetKey;
        result[i] = path_type;
    }
    return result;
}

template <PathType path_type>
constexpr auto convert(KeyOrIndex path) {
    if constexpr (path_type == PathType::ObjetKey) {
        return std::get<std::string_view>(path);
    } else {
        return std::get<size_t>(path);
    }
}

template <size_t... Is, typename WrappedStringLiteral>
constexpr auto parse_json_impl(WrappedStringLiteral fn, std::index_sequence<Is...>) {
    constexpr auto paths = parse_json_paths(fn);
    constexpr auto path_types = get_path_types(paths);
    return std::make_tuple(convert<path_types[Is]>(paths[Is])...);
}

template <typename WrappedStringLiteral>
constexpr auto parse_json(WrappedStringLiteral fn) {
    constexpr auto size = count_json_paths(fn());
    return parse_json_impl(fn, std::make_index_sequence<size>{});
}

template <typename WrappedStringLiteral>
boost::json::value& query_json(boost::json::value& json, WrappedStringLiteral fn) {
    constexpr auto keys = parse_json(fn);
    auto* ref = &json;
    boost::hana::for_each(keys, [&](auto x) {
        ref = &(ref->at(x));
    });
    return *ref;
}

template <typename WrappedStringLiteral>
const boost::json::value& query_json(const boost::json::value& json, WrappedStringLiteral fn) {
    constexpr auto keys = parse_json(fn);
    const auto* ref = &json;
    boost::hana::for_each(keys, [&](auto x) {
        ref = &(ref->at(x));
    });
    return *ref;
}

} // namespace json_query_internal

#define QUERY_JSON_IMPL_2(json, path)                   \
json_query_internal::query_json(                        \
    json,                                               \
    []() constexpr { return std::string_view(#path); })

#define QUERY_JSON_IMPL_3(json, path, type)             \
json_query_internal::query_json(                        \
    json,                                               \
    []() constexpr { return std::string_view(#path); }  \
).as_##type()

#define QUERY_JSON(...) BOOST_PP_OVERLOAD(QUERY_JSON_IMPL_, __VA_ARGS__)(__VA_ARGS__)

#endif // JSON_QUERY_MACRO_HPP_
