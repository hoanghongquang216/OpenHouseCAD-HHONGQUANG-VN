# Build and test

## Requirements

- CMake 3.25 or newer.
- A C++23 compiler and standard library with support for `std::expected`,
  `std::format`, `std::byteswap`, and `std::unreachable` (GCC 13+ or
  Clang 17+ recommended; CI currently verifies GCC 13 and Clang 18 on
  Ubuntu 24.04).

## Commands

Configure, build, and run the test suite from the repository root:

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

To match CI's strictness (warnings treated as errors):

```bash
cmake -S . -B build -DOPENHOUSE_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
```

## Building the application layer (not yet implemented)

`modules/app` (Qt6-based; see
`docs/ARCHITECTURE_DECISION_RECORDS/ADR-0003-windowing-gui-stack.md`) is
scaffold-only and skipped by default. Enabling it without Qt6 installed
fails with a clear error rather than building anything -- see
`docs/QT_INTEGRATION_CHECKLIST.md` before turning this on:

```bash
cmake -S . -B build -DOPENHOUSE_BUILD_APP=ON
```

## Continuous integration

`.github/workflows/ci.yml` currently runs on `ubuntu-24.04` only (GCC 13
and Clang 18, Debug and Release), with warnings treated as errors and
demo executables executed (not just compiled) as part of the run.

**Windows CI is not yet implemented.** An earlier version of this
document described a Windows Server 2022 CI job; at the time of writing
that job was never actually committed to this repository (verified
against `git log` -- no prior commit touched `.github/workflows/`), so
this section previously documented an intention rather than a shipped
workflow. Per `docs/ROADMAP_EXECUTION.md`'s v0.1 Alpha criteria (which
does require a successful Windows build), Windows CI is deliberately
deferred until the project is closer to that milestone, rather than
maintaining a second OS in the matrix ahead of having a Windows-specific
feature to validate.
