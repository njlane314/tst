# tst

`tst` is a tiny C++20 test helper for hand-written test programs: one header,
one `main`, and ordinary C++ control flow.

```cpp
#include <tst.hpp>

#include <stdexcept>
#include <string_view>

int main()
{
    const auto parse = [](std::string_view input) {
        if (input.empty()) throw std::invalid_argument{"empty input"};
    };

    return tst::run(
        tst::test{"adds values", [] {
            TST(2 + 3 == 5);
        }},
        tst::test{"rejects empty input", [&] {
            TST_THROWS_AS(parse(""), std::invalid_argument);
        }});
}
```

```text
PASS  adds values
PASS  rejects empty input

2 passed, 0 failed
```

If `parse("")` returned normally, the same program would report:

```text
PASS  adds values
FAIL  rejects empty input
test.cpp:17: parse("") did not throw std::invalid_argument

1 passed, 1 failed
```

`tst::run` executes its named tests from left to right and returns `0` only
when all pass. A failed check stops its test, but later tests still run. An
assertion or missing expected exception is `FAIL`; an unexpected exception is
`ERROR`. Both produce exit status `1`, and all report lines use standard output.

`TST(expression)` evaluates its expression once and reports its text, file, and
line on failure. `TST_THROWS_AS(expression, Error)` requires the expression to
throw `Error`. The underlying `tst::check` and `tst::throws<Error>` functions are
available when a macro is undesirable; their messages are optional.

The preprocessor only understands parentheses when separating macro arguments.
Wrap an exception expression containing a top-level template or braced comma,
for example `TST_THROWS_AS((make<std::pair<int, int>>()), parse_error)`.

Tests remain ordinary C++: use functions and RAII for setup, lambda captures for
shared state, loops for data-driven checks, and templates for testing multiple
types. The library requires C++ exceptions and does not support
`-fno-exceptions`.

`tst` does not transport exceptions between threads. A worker failure must be
transported with a future or `std::exception_ptr` and rethrown inside its named
test; an exception escaping a thread still terminates the process.

There is no global registry, generated `main`, nonfatal assertion state, matcher
or value-formatting hierarchy, filtering, colour, reporter system, mocking, or
benchmarking support.

```sh
c++ -std=c++20 -I/path/to/tst test.cpp -o test
make check
make install PREFIX="$HOME/.local"
```

`PREFIX`, `DESTDIR`, and `INCLUDEDIR` are supported. The library is the single
header `tst.hpp`, has no runtime dependencies, and is distributed under the MIT
License.
