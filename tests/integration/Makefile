# compiler and compiler flags (we use libasan for address detection and leaks).
cflags := -g -O0 -Wall -Wextra -std=c17 # -fsanitize=address,leak,undefined -fno-omit-frame-pointer
# ^ removing comment and including libasan will cause errors for
#	non-native architectures being ran in qemu, you can also enable this
#	by setting use_asan=true, when compiling with this Makefile.
use_asan := false

# architecture defs.
arch ?= $(shell uname -m).
ifeq ($(arch),aarch64)
  cc := aarch64-linux-gnu-gcc
endif
ifeq ($(arch),arm32)
  cc := arm-linux-gnueabihf-gcc
endif
ifeq ($(arch),x86)
  cc := i686-linux-gnu-gcc
endif
ifeq ($(arch),x86_64)
  cc := gcc
endif

# asan usage.
ifeq ($(use_asan),true)
	cflags += -fsanitize=address,leak,undefined -fno-omit-frame-pointer
endif

# dirs relative to tests/integration/
root_dir  := ../..
src_dir   := .
build_dir := $(root_dir)/build/$(arch)/tests/integration
bin_dir   := $(root_dir)/bin/$(arch)
test_bin_dir := $(bin_dir)/tests/integration

# library we link against.
lib_name := tapi
lib_file := $(bin_dir)/lib$(lib_name).so

# include paths (public headers).
cflags += -I$(bin_dir)/include

# link flags:
# -L../../bin/$(arch)/lib so -ltapi finds libtapi.so
# rpath=$$ORIGIN/../.. so test binaries (in bin/$(arch)/tests/integration/) find libtapi.so (in bin/$(arch)/) at runtime
ldflags := -L$(bin_dir) -Wl,-rpath,'$$ORIGIN/../..'
ldlibs  := -l$(lib_name)

# each test .c becomes its own executable in bin/$(arch)/tests/integration/
test_srcs := $(shell find $(src_dir) -maxdepth 1 -name '*.c')
test_objs := $(patsubst $(src_dir)/%.c,$(build_dir)/%.o,$(test_srcs))
test_bins := $(patsubst $(src_dir)/%.c,$(test_bin_dir)/%,$(test_srcs))

.PHONY: all
all: $(test_bins)

$(test_bin_dir)/%: $(build_dir)/%.o $(lib_file)
	@mkdir -p $(@D)
	$(cc) $(cflags) $< -o $@ $(ldflags) $(ldlibs)

$(build_dir)/%.o: $(src_dir)/%.c
	@mkdir -p $(dir $@)
	$(cc) $(cflags) -c $< -o $@

.PHONY: run
run: all
	@set -e; for t in $(test_bins); do echo "==> $$t"; $$t; done

.PHONY: clean
clean:
	rm -rf $(build_dir)
	rm -f $(test_bins)