#include "tst.hpp"

#include <stdexcept>
#include <string>

int main()
{
    bool checked = false;
    if (tst::run("tst", [&] {
            tst::check(true, "true check");
            tst::throws<std::runtime_error>([] { throw std::runtime_error{"expected"}; },
                                            "expected runtime_error");
            try {
                tst::check(false, "check message");
            } catch (const std::runtime_error& error) {
                checked = std::string{error.what()} == "check message";
            }
            tst::check(checked, "failed check keeps its message");
        }) != 0) {
        return 1;
    }

    return tst::run("expected failure", [] { tst::check(false, "failure"); }) == 1 ? 0 : 1;
}
