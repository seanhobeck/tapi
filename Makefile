# compilation data and commands used.
rm := rm -rf
cp := cp
mkdir := mkdir -p
pkg_config := pkg-config

# architecture defs. todo; update for non-linux architectures
arch ?= $(shell uname -m)
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
  cc := x86_64-linux-gnu-gcc
endif

# compilation, release, and asan flags.
cflags := -std=c99 -Wall -Wextra -g -O0 -fPIC
release ?= 0
ifeq ($(release),1)
  cflags := -std=c17 -Wall -Wextra -O2 -fPIC
endif
use_asan := false

# directories for source files, headers, build directories and relative build directories.
src_dir := src
include_dir := include
test_dir := tests/
rel_build_dir := build/
build_dir := $(rel_build_dir)$(arch)
rel_bin_dir := bin/
bin_dir := $(rel_bin_dir)$(arch)
bin_inc_dir := $(bin_dir)/include
bin_pkg_dir := $(bin_dir)/lib/pkgconfig

# vendor directories.
vendor_include := vendor/build/$(arch)/include
vendor_lib := vendor/build/$(arch)/lib

# shared object names.
lib_name := tapi
lib_file := $(bin_dir)/lib$(lib_name).so

# vendor flags. prefer pkg-config (a system-wide capstone install), otherwise
# fall back to the vendored static libcapstone.a built by scripts/get-deps. this
# removes the hard dependency on pkg-config being installed on the build host.
capstone_cflags := $(shell $(pkg_config) --cflags capstone 2>/dev/null)
capstone_libs   := $(shell $(pkg_config) --libs   capstone 2>/dev/null)
have_capstone := 0
ifneq ($(strip $(capstone_libs)),)
  have_capstone := 1
else ifneq ($(wildcard $(vendor_lib)/libcapstone.a),)
  # no pkg-config / no system capstone; link the vendored static archive in
  # (it is built -fPIC, so it folds cleanly into the shared lib_file).
  capstone_cflags := -I$(vendor_include) -I$(vendor_include)/capstone
  capstone_libs   := -L$(vendor_lib) -l:libcapstone.a
  have_capstone := 1
endif

# includes and appending vendor flags to cflags.
cflags += -I$(include_dir)
includes := $(shell find $(include_dir) -type d 2>/dev/null)
includes += $(shell find $(src_dir) -type d 2>/dev/null)
cflags += $(addprefix -I,$(includes))
ifneq ($(wildcard $(vendor_include)),)
  cflags += -I$(vendor_include)
endif
ifeq ($(have_capstone),1)
  cflags += $(capstone_cflags)
endif

# search for source objects.
srcs := $(shell find $(src_dir) -name '*.c')
objs := $(patsubst $(src_dir)/%.c,$(build_dir)/%.o,$(srcs))

# linker flags, check vendor.
ldflags := -shared -Wl,-rpath,'$$ORIGIN'
ldlibs :=
ifneq ($(wildcard $(vendor_lib)),)
  ldflags += -L$(vendor_lib)
endif
ifeq ($(have_capstone),1)
  ldlibs += $(capstone_libs)
endif

# project headers and destination directories.
project_headers := $(shell find $(include_dir) -name '*.h')
project_headers_dst := $(patsubst $(include_dir)/%,$(bin_inc_dir)/%,$(project_headers))
pc_file := $(bin_pkg_dir)/$(lib_name).pc
prefix ?= /usr/local
includedir ?= $(prefix)/include
libdir ?= $(prefix)/lib
pkgconfigdir ?= $(libdir)/pkgconfig
destdir ?=

# test source and binaries (integration & unit tests).
test_integration_dir := tests/integration/
test_unit_dir := tests/unit/
test_integration_srcs := $(shell find $(test_integration_dir) -maxdepth 1 -name '*.c' 2>/dev/null)
test_unit_srcs := $(shell find $(test_unit_dir) -maxdepth 1 -name '*.c' 2>/dev/null)
test_bins := $(patsubst $(test_integration_dir)%.c,$(bin_dir)/tests/integration/%,$(test_integration_srcs))
test_bins += $(patsubst $(test_unit_dir)%.c,$(bin_dir)/tests/unit/%,$(test_srcs))

# test flags.
test_cflags := -std=c17 -Wall -Wextra -g -O0 -I$(bin_inc_dir)
test_ldflags := -L$(bin_dir) -Wl,-rpath,'$$ORIGIN/../..'
test_ldlibs := -l$(lib_name)

.PHONY: all
all: stage_headers $(pc_file) $(lib_file)

$(build_dir)/%.o: $(src_dir)/%.c
	@$(mkdir) $(dir $@)
	$(cc) $(cflags) -c $< -o $@

$(lib_file): $(objs)
	@$(mkdir) $(dir $@)
	$(cc) $(cflags) $(ldflags) $^ -o $@ $(ldlibs)

.PHONY: stage_headers
stage_headers: $(project_headers_dst)

$(bin_inc_dir)/%: $(include_dir)/%
	@$(mkdir) $(dir $@)
	$(cp) $< $@

$(pc_file): $(lib_file)
	@$(mkdir) $(bin_pkg_dir)
	@printf "prefix=%s\n" "$(prefix)" >  $(pc_file)
	@printf "includedir=%s\n" "$(includedir)" >> $(pc_file)
	@printf "libdir=%s\n" "$(libdir)" >> $(pc_file)
	@printf "\n" >> $(pc_file)
	@printf "name: %s\n" "$(lib_name)" >> $(pc_file)
	@printf "description: A testing framework in C17 with runtime-based mocking for various architectures. \n" >> $(pc_file)
	@printf "version: v1.0\n" >> $(pc_file)
	@printf "cflags: -I$${includedir}\n" >> $(pc_file)
	@printf "libs: -L$${libdir} -l%s\n" "$(lib_name)" >> $(pc_file)

.PHONY: test
test: all $(test_bins)

$(bin_dir)/tests/integration/%: $(test_integration_dir)%.c $(lib_file) stage_headers
	@$(mkdir) $(dir $@)
	$(cc) $(test_cflags) $< -o $@ $(test_ldflags) $(test_ldlibs)

.PHONY: install
install: all
	@$(mkdir) "$(destdir)$(libdir)"
	@$(cp) $(lib_file) "$(destdir)$(libdir)/"
	@$(mkdir) "$(destdir)$(includedir)"
	@$(cp) -a "$(bin_inc_dir)/." "$(destdir)$(includedir)/"
	@$(mkdir) "$(destdir)$(pkgconfigdir)"
	@$(cp) $(pc_file) "$(destdir)$(pkgconfigdir)/"

.PHONY: uninstall
uninstall:
	@$(rm) "$(destdir)$(libdir)/lib$(lib_name).so"
	@$(rm) "$(destdir)$(pkgconfigdir)/$(lib_name).pc"

.PHONY: test_native
test_native: all
	$(MAKE) -C tests/integration arch=$(arch) use_asan=$(use_asan)
	$(MAKE) -C tests/unit arch=$(arch) use_asan=$(use_asan)

.PHONY: test_all_arch
test_all_arch:
	$(MAKE) all test_native arch=x86_64 use_asan=false
	$(MAKE) all test_native arch=aarch64 use_asan=false
	$(MAKE) all test_native arch=arm32 use_asan=false
	$(MAKE) all test_native arch=x86 use_asan=false

.PHONY: abs_clean
abs_clean:
	$(rm) $(bin_dir)
	$(rm) $(build_dir)

.PHONY: clean_arch
clean_arch:
	$(MAKE) abs_clean arch=x86_64
	$(MAKE) abs_clean arch=aarch64
	$(MAKE) abs_clean arch=arm32
	$(MAKE) abs_clean arch=x86

.PHONY: clean
clean:
	$(rm) $(rel_bin_dir)
	$(rm) $(rel_build_dir)
