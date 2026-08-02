# tst

[![Build](https://github.com/njlane314/tst/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/njlane314/tst/actions/workflows/ci.yml)
![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C)

`tst` is a tiny C++20 test helper for hand-written test programs: one header,
one `main`, and ordinary C++ control flow.

## USE

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
        TST_CASE("adds values", TST(2 + 3 == 5)),
        TST_CASE("rejects empty input",
                 TST_THROWS_AS(parse(""), std::invalid_argument)));
}
```

```text
PASS  adds values
PASS  rejects empty input

2 passed, 0 failed
```

`tst::run` executes its named tests from left to right and returns `0` only
when all pass. Tests remain ordinary C++: use functions and RAII for setup,
lambda captures for shared state, loops for data-driven checks, and templates
for testing multiple types.

## FAILURES

If `parse("")` returned normally, the same program would report:

```text
PASS  adds values
FAIL  rejects empty input
test.cpp:15: parse("") did not throw std::invalid_argument

1 passed, 1 failed
```

A failed check stops its test, but later tests still run. An assertion or
missing expected exception is `FAIL`; an unexpected standard or non-standard
exception is `ERROR`. Both produce exit status `1`, and all reports use
standard output.

An assertion raised inside `tst::throws` remains a test failure rather than
satisfying the expected-exception check. A different exception escapes to the
test runner and is reported as `ERROR`.

## API

`TST_CASE(name, body)` creates a named reference-capturing test for immediate
use with `tst::run`. Do not retain it beyond the lifetime of anything used by
its body. `TST(expression)` evaluates its expression once and reports its text,
file, and line on failure. `TST_THROWS_AS(expression, Error)` evaluates its
expression once and requires it to throw `Error`.

The underlying `tst::test`, `tst::check`, and `tst::throws<Error>` interfaces
are available when a macro is undesirable. Messages are optional, and direct
checks capture their call-site source location by default.

## DESIGN

A test is an ordinary named callable passed explicitly to `tst::run`. There is
no global registry, generated `main`, or static test registration. The runner
executes callables in order, isolates each failure, prints a final count, and
returns a process status suitable for build tools and CI.

The single `tst.hpp` header has no runtime dependencies beyond the C++ standard
library.

## LIMITS

The library requires a C++20 compiler with exceptions enabled and does not
support `-fno-exceptions`.

The preprocessor only understands parentheses when separating macro arguments.
Wrap an exception expression containing a top-level template or braced comma,
for example `TST_THROWS_AS((make<std::pair<int, int>>()), parse_error)`.

`tst` does not transport exceptions between threads. A worker failure must be
transported with a future or `std::exception_ptr` and rethrown inside its named
test; an exception escaping a thread still terminates the process.

There is no nonfatal assertion state, matcher or value-formatting hierarchy,
filtering, colour, reporter system, mocking, or benchmarking support.

## INSTALL

Vendor the repository and include the header directly:

```sh
git submodule add https://github.com/njlane314/tst vendor/tst
c++ -std=c++20 -Ivendor/tst test.cpp -o test
./test
```

Or install the header under a chosen prefix:

```sh
make install PREFIX="$HOME/.local"
c++ -std=c++20 -I"$HOME/.local/include" test.cpp -o test
```

`PREFIX`, `DESTDIR`, and `INCLUDEDIR` are supported.

## CHECKS

```sh
make check
```

The checks compile with warnings as errors and run on Ubuntu and macOS in CI.

## LICENSE

[MIT](LICENSE)
