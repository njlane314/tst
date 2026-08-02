#include "tst.hpp"

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

bool included_from_another_translation_unit();

namespace {

struct explicit_condition {
    bool value;
    int* evaluations;

    explicit operator bool() const noexcept
    {
        ++*evaluations;
        return value;
    }
};

struct captured_run {
    int status;
    std::string output;
    std::string error;
};

class output_capture {
public:
    output_capture()
        : previous_output_{std::cout.rdbuf(output_.rdbuf())},
          previous_error_{std::cerr.rdbuf(error_.rdbuf())}
    {}
    ~output_capture()
    {
        std::cout.rdbuf(previous_output_);
        std::cerr.rdbuf(previous_error_);
    }

    std::string str() const { return output_.str(); }
    std::string error() const { return error_.str(); }

private:
    std::ostringstream output_;
    std::ostringstream error_;
    std::streambuf* previous_output_;
    std::streambuf* previous_error_;
};

template<class... Tests>
captured_run capture(Tests&&... tests)
{
    int status;
    std::string output;
    std::string error;
    {
        output_capture capture;
        status = tst::run(std::forward<Tests>(tests)...);
        output = capture.str();
        error = capture.error();
    }
    return {status, std::move(output), std::move(error)};
}

void throw_runtime_error() { throw std::runtime_error{"expected"}; }
void throw_logic_error() { throw std::logic_error{"wrong exception"}; }
void throw_int() { throw 7; }

template<class, class>
struct typed_error {};

void throw_typed_error() { throw typed_error<int, int>{}; }

} // namespace

int main()
{
    int case_runs = 0;
    return tst::run(
        TST_CASE("checks ordinary conditions",
            TST(++case_runs == 1);
            int evaluations = 0;
            TST(++evaluations == 1);
            TST(evaluations == 1);
            int conversions = 0;
            TST(explicit_condition{true, &conversions});
            tst::check(explicit_condition{true, &conversions});
            TST(conversions == 2);
            TST(std::is_same_v<std::pair<int, int>, std::pair<int, int>>)),
        tst::test{"supports move-only captures", [value = std::make_unique<int>(42)] {
            TST(*value == 42);
        }},
        tst::test{"checks typed exceptions", [] {
            int evaluations = 0;
            tst::throws<std::runtime_error>(throw_runtime_error);
            tst::throws<int>([] { throw 7; });
            TST_THROWS_AS((++evaluations, throw_runtime_error()), std::runtime_error);
            TST_THROWS_AS(throw_typed_error(), typed_error<int, int>);
            TST(evaluations == 1);
        }},
        tst::test{"isolates tests and reports stable results", [] {
            int evaluations = 0;
            int reached = 0;
            int failure_line = 0;
            auto failure = tst::test{"fails", [&] {
                ++reached;
                failure_line = __LINE__ + 1;
                TST(++evaluations == 0);
                ++reached;
            }};

            const auto result = capture(
                tst::test{"passes", [&] { ++reached; }}, failure,
                tst::test{"throws", [] { throw std::runtime_error{"boom"}; }},
                tst::test{"throws non-standard", [] { throw 7; }},
                tst::test{"continues", [&] { ++reached; }});

            const auto expected =
                std::string{"PASS  passes\nFAIL  fails\n"} + __FILE__ + ':' +
                std::to_string(failure_line) +
                ": ++evaluations == 0\nERROR throws\nunexpected exception: boom\n"
                "ERROR throws non-standard\nunexpected non-standard exception\n"
                "PASS  continues\n\n2 passed, 3 failed\n";

            TST(result.status == 1);
            TST(evaluations == 1);
            TST(reached == 3);
            TST(result.output == expected);
            TST(result.error.empty());
        }},
        tst::test{"reports exception expectation failures", [] {
            int evaluations = 0;
            int missing_line = 0;
            auto missing = tst::test{"missing exception", [&] {
                missing_line = __LINE__ + 1;
                TST_THROWS_AS(++evaluations, std::runtime_error);
            }};

            const auto result = capture(
                missing,
                tst::test{"wrong exception", [] {
                    TST_THROWS_AS(throw_logic_error(), std::runtime_error);
                }},
                tst::test{"wrong non-standard exception", [] {
                    TST_THROWS_AS(throw_int(), std::runtime_error);
                }},
                tst::test{"continues after exception failures", [] {}});

            const auto expected =
                std::string{"FAIL  missing exception\n"} + __FILE__ + ':' +
                std::to_string(missing_line) +
                ": ++evaluations did not throw std::runtime_error\n"
                "ERROR wrong exception\nunexpected exception: wrong exception\n"
                "ERROR wrong non-standard exception\nunexpected non-standard exception\n"
                "PASS  continues after exception failures\n\n1 passed, 3 failed\n";

            TST(evaluations == 1);
            TST(result.status == 1);
            TST(result.output == expected);
            TST(result.error.empty());
        }},
        tst::test{"does not swallow framework failures", [] {
            int failure_line = 0;
            const auto result = capture(tst::test{"nested check", [&] {
                tst::throws<std::runtime_error>([&] {
                    failure_line = __LINE__ + 1;
                    tst::check(false, "inner failure");
                });
            }});

            const auto expected =
                std::string{"FAIL  nested check\n"} + __FILE__ + ':' +
                std::to_string(failure_line) + ": inner failure\n\n0 passed, 1 failed\n";

            TST(result.status == 1);
            TST(result.output == expected);
            TST(result.error.empty());
        }},
        tst::test{"reports useful default messages", [] {
            int check_line = 0;
            int throws_line = 0;
            const auto result = capture(
                tst::test{"default check message", [&] {
                    check_line = __LINE__ + 1;
                    tst::check(false);
                }},
                tst::test{"default exception message", [&] {
                    throws_line = __LINE__ + 1;
                    tst::throws<std::runtime_error>([] {});
                }});

            const auto expected =
                std::string{"FAIL  default check message\n"} + __FILE__ + ':' +
                std::to_string(check_line) +
                ": check failed\nFAIL  default exception message\n" + __FILE__ + ':' +
                std::to_string(throws_line) +
                ": expected exception was not thrown\n\n0 passed, 2 failed\n";

            TST(result.status == 1);
            TST(result.output == expected);
            TST(result.error.empty());
        }},
        tst::test{"includes cleanly in another translation unit", [] {
            TST(included_from_another_translation_unit());
        }});
}
