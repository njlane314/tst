# tst

`tst` is a deliberately small C++20 test helper. The entire library is the
single header `tst.hpp`.

```cpp
#include <tst.hpp>

int main()
{
    return tst::run("widget tests", [] {
        tst::check(2 + 2 == 4, "addition");
        tst::throws<std::runtime_error>(
            [] { throw std::runtime_error{"expected"}; },
            "operation should throw");
    });
}
```

`tst::check` throws `std::runtime_error` when a condition is false.
`tst::throws<Error>` requires an action to throw `Error`. `tst::run` prints one
result and converts uncaught exceptions into a failing exit status. There is no
global registry, matcher hierarchy, value formatter, filtering, or macro DSL.
Checks can run on worker threads when their exceptions are transported back to
the runner.

```sh
make check
make install PREFIX="$HOME/.local"
```

`PREFIX`, `DESTDIR`, and `INCLUDEDIR` are supported. The library has no runtime
dependencies and is distributed under the MIT License.
