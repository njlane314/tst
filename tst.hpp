// tst.hpp - tiny C++20 tests with ordinary control flow
// Copyright (c) 2026 njlane314
// SPDX-License-Identifier: MIT
// https://github.com/njlane314/tst

#pragma once

// Standard-library dependencies ------------------------------------------------

#include <exception>
#include <iostream>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace tst {

// Internal failure transport ---------------------------------------------------

namespace detail {

// Assertions use a private sentinel exception to leave the current case while
// retaining its call site. run_one catches it before classifying user exceptions.
struct failure : std::runtime_error {
    std::source_location location;

    failure(std::string_view message, std::source_location location)
        : std::runtime_error{std::string{message}}, location{location}
    {}
};

template<class Action>
bool run_one(std::string_view name, Action&& action)
{
    // Convert one callable into PASS, FAIL, or ERROR. Caught failures and
    // exceptions do not prevent later cases from running.
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

// Public assertions ------------------------------------------------------------

template<class Condition>
void check(Condition&& condition, std::string_view message = "check failed",
           std::source_location location = std::source_location::current())
{
    // The default argument captures the caller rather than this function body.
    // A false boolean-like value leaves the current case through failure.
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
        // Do not let throws<std::exception> mistake an assertion for success.
        throw;
    } catch (const Error&) {
        // Normal C++ catch rules apply, so derived error types also satisfy it.
        return;
    }
    // Other exceptions propagate to run_one as ERROR; normal return is FAIL.
    throw detail::failure{message, location};
}

// Test model and execution ------------------------------------------------------

// A test owns an ordinary name and callable; no registry or static setup exists.
template<class Action>
struct test {
    std::string name;
    Action action;
};

template<class... Tests>
int run(Tests&&... tests)
{
    // The comma fold executes every case once, left-to-right. The returned status
    // is directly suitable for main(): zero only when every case passed.
    int passed = 0;
    int failed = 0;
    ((detail::run_one(tests.name, std::forward<Tests>(tests).action) ? ++passed : ++failed),
     ...);
    std::cout << '\n' << passed << " passed, " << failed << " failed\n";
    return failed != 0;
}

} // namespace tst

// Convenience macros -----------------------------------------------------------

// Cases capture their surrounding test data by reference. Pass the temporary
// directly to run() rather than storing it beyond those references' lifetimes.
#define TST_CASE(name, ...) ::tst::test{name, [&] { __VA_ARGS__; }}
// The assertion expression is evaluated once; stringization supplies its text.
#define TST(...) ::tst::check(static_cast<bool>((__VA_ARGS__)), #__VA_ARGS__)
// The expression runs once inside its case. A variadic type argument permits
// expected exception types whose template spelling itself contains commas.
#define TST_THROWS_AS(expression, ...)                                                     \
    ::tst::throws<__VA_ARGS__>([&] { static_cast<void>(expression); },                     \
                               #expression " did not throw " #__VA_ARGS__)
