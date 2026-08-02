// SPDX-License-Identifier: MIT
#pragma once

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace tst {

inline void check(bool condition, std::string_view message)
{
    if (!condition) throw std::runtime_error{std::string{message}};
}

template<class Error = std::exception, class Action>
void throws(Action&& action, std::string_view message)
{
    try {
        std::forward<Action>(action)();
    } catch (const Error&) {
        return;
    }
    throw std::runtime_error{std::string{message}};
}

template<class Action>
int run(std::string_view name, Action&& action) noexcept
{
    try {
        std::forward<Action>(action)();
        std::cout << name << ": ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << name << ": " << error.what() << '\n';
    } catch (...) {
        std::cerr << name << ": failed\n";
    }
    return 1;
}

} // namespace tst
