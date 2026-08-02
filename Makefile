CXX ?= c++
CXXFLAGS ?= -std=c++20 -Wall -Wextra -Wpedantic -Werror
PREFIX ?= /usr/local
INCLUDEDIR ?= $(PREFIX)/include

ifeq ($(origin CXX),default)
ifneq ($(wildcard /opt/homebrew/opt/llvm/bin/clang++),)
CXX := /opt/homebrew/opt/llvm/bin/clang++
else ifneq ($(wildcard /usr/local/opt/llvm/bin/clang++),)
CXX := /usr/local/opt/llvm/bin/clang++
endif
endif

build := .build

.DEFAULT_GOAL := check
.PHONY: check install clean

check: $(build)/test
	@$<

$(build)/test: test.cpp include_test.cpp tst.hpp | $(build)
	$(CXX) $(CXXFLAGS) -I. test.cpp include_test.cpp -o $@

$(build):
	mkdir -p $@

install:
	install -d "$(DESTDIR)$(INCLUDEDIR)"
	install -m 0644 tst.hpp "$(DESTDIR)$(INCLUDEDIR)/tst.hpp"

clean:
	rm -rf $(build)
