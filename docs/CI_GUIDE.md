# OpenHouseCAD CI Guide

## Purpose

Explain how continuous integration verifies changes before integration.

## Local Verification

Recommended workflow:

```text
Configure
   |
   v
Build
   |
   v
Run Tests
```

Commands:

```bash
cmake -S . -B build -G Ninja -DOPENHOUSECAD_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## CI Pipeline

```text
Pull Request
      |
      v
GitHub Actions
      |
      +-- Configure CMake
      +-- Build Targets
      +-- Execute Tests
```

## Failure Handling

When CI fails:

1. Check compiler errors.
2. Check missing module dependencies.
3. Check failed tests.
4. Fix locally before merging.

## Multi AI Workflow

- Core AI owns architecture changes.
- Review AI validates code quality and tests.
- Build AI maintains CI and build reliability.
