# tapi — A testing framework for C17

A testing framework for C17(_or 99_) using libcapstone. `tapi` is a simple, yet powerful testing 
framework that supports unit testing, mocking, and stream capturing for various architectures.  


---

## Features

- Unit testing
- Easy logging and debugging
- Grouping tests into suites
- POSIX file stream capturing
- Proper assertion and error handling
- Runtime mocking by patching call targets

---

### Dependencies

- A compiler with C99 support(_minimum GCC 4.5+, but should support C17 as well_)
- `libcapstone` (_included as a submodule_)

---

## Build

To get started with `tapi`, clone the repository and build it using make.

```bash
# Clone the repository.
git clone https://github.com/seanhobeck/tapi.git
cd tapi

# Gather and build dependencies.
sudo ./get-deps

# Build tapi for a specific architecture,
make all release=1 arch=target_architecture # x86_64, x86, arm32, arm64

# or build it for your native architecture.
make all release=1
```

[//]: # (Support for C++)

[//]: # ()
[//]: # (Multi-threaded testing using mocks &#40;_ie. mocking an application using multiple threads_&#41;)

[//]: # ()
[//]: # (Support for even more little-endian architectures &#40;_e.g. POWERPC, and RISC-V for both 32-bit )

[//]: # ()
[//]: # (  and 64-bit_&#41;)

