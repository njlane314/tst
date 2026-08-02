// SPDX-License-Identifier: MIT
#pragma once

#include <exception>
#include <iostream>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace tst {

namespace detail {

struct failure : std::runtime_error {
    std::source_location location;

    failure(std::string_view message, std::source_location location)
        : std::runtime_error{std::string{message}}, location{location}
    {}
};

template<class Action>
bool run_one(std::string_view name, Action&& action)
{
    try {
        std::forward<Action>(action)();
    } catch (const failure& error) {
        std::cout << "FAIL  " << name << '\n'
                  << error.location.file_name() << ':' << error.location.line() << ": "
                  << error.what() << '\n';
        return false;
    } catch (const std::exception& error) {
        std::cout << "ERROR " << name << "\nunexpected exception: " << error.what() << '\n';
        return false;
    } catch (...) {
        std::cout << "ERROR " << name << "\nunexpected non-standard exception\n";
        return false;
    }
    std::cout << "PASS  " << name << '\n';
    return true;
}

} // namespace detail

template<class Condition>
void check(Condition&& condition, std::string_view message = "check failed",
           std::source_location location = std::source_location::current())
{
    if (!static_cast<bool>(std::forward<Condition>(condition)))
        throw detail::failure{message, location};
}

template<class Error, class Action>
void throws(Action&& action, std::string_view message = "expected exception was not thrown",
            std::source_location location = std::source_location::current())
{
    try {
        std::forward<Action>(action)();
    } catch (const detail::failure&) {
        throw;
    } catch (const Error&) {
        return;
    }
    throw detail::failure{message, location};
}

template<class Action>
struct test {
    std::string name;
    Action action;
};

template<class... Tests>
int run(Tests&&... tests)
{
    int passed = 0;
    int failed = 0;
    ((detail::run_one(tests.name, std::forward<Tests>(tests).action) ? ++passed : ++failed),
     ...);
    std::cout << '\n' << passed << " passed, " << failed << " failed\n";
    return failed != 0;
}

} // namespace tst

#define TST_CASE(name, ...) ::tst::test{name, [&] { __VA_ARGS__; }}
#define TST(...) ::tst::check(static_cast<bool>((__VA_ARGS__)), #__VA_ARGS__)
#define TST_THROWS_AS(expression, ...)                                                     \
    ::tst::throws<__VA_ARGS__>([&] { static_cast<void>(expression); },                     \
                               #expression " did not throw " #__VA_ARGS__)
