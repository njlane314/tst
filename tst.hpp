// SPDX-License-Identifier: MIT
// tst.hpp is a tiny C++20 test helper for ordinary hand-written test programs.
// Tests are callables; tst supplies assertions, reporting, and a process status.
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

// This internal control-flow exception stops only the currently running test.
struct failure : std::runtime_error {
    std::source_location location;

    failure(std::string_view message, std::source_location location)
        : std::runtime_error{std::string{message}}, location{location}
    {}
};

template<class Action>
bool run_one(std::string_view name, Action&& action)
{
    // Convert assertion and unexpected-exception outcomes into one test result.
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
    // The default location identifies the caller rather than this function body.
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
        // An assertion inside the action remains a failure of the current test.
        throw;
    } catch (const Error&) {
        // Only the requested error type satisfies the expectation.
        return;
    }
    // Returning normally is a failure at the expectation's call site.
    throw detail::failure{message, location};
}

// A test is an ordinary named callable; no registry or static setup is involved.
template<class Action>
struct test {
    std::string name;
    Action action;
};

template<class... Tests>
int run(Tests&&... tests)
{
    // Execute left-to-right, continue after failures, and return a process status.
    int passed = 0;
    int failed = 0;
    ((detail::run_one(tests.name, std::forward<Tests>(tests).action) ? ++passed : ++failed),
     ...);
    std::cout << '\n' << passed << " passed, " << failed << " failed\n";
    return failed != 0;
}

} // namespace tst

// Cases capture by reference and should be passed directly to run().
#define TST_CASE(name, ...) ::tst::test{name, [&] { __VA_ARGS__; }}
// Each assertion expression is evaluated once; stringization supplies its text.
#define TST(...) ::tst::check(static_cast<bool>((__VA_ARGS__)), #__VA_ARGS__)
// The expected-exception expression is evaluated once inside its case action.
#define TST_THROWS_AS(expression, ...)                                                     \
    ::tst::throws<__VA_ARGS__>([&] { static_cast<void>(expression); },                     \
                               #expression " did not throw " #__VA_ARGS__)
