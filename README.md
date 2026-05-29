# tapi — A testing framework for C17

A testing framework for C17(_or 99_) using libcapstone. `tapi` is a simple, yet powerful testing 
framework that supports unit testing, mocking, and stream capturing for various architectures.  


---

## Features

- Support for both unit, integration, and e2e testing.
- Easy logging and debugging for fast test-driven development.
- POSIX file stream capturing for monitoring and debugging outputs.
- Proper assertion and error handling for each test case.
- Mocking during runtime by patching call targets.

---

### Dependencies

- A compiler with C17 support(_minimum GCC 4.5+, but should support C99 as well_)
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

## Documentation

To view documentation for `tapi` there are a few options:
- View via [Github Pages](https://seanhobeck.github.io/tapi).
- Generate documentation by running `scripts/get-docs` from the cloned repository.
- Locally view the documentation by either generating it (_see above_) or using the downloaded 
  tarball from the [releases page](https://github.com/seanhobeck/tapi/releases), and then 
  opening either `build/doc/html/index.html` or `docs/index.html` with your favorite web browser.

