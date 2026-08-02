# ADR 0002: C++23 Language Standard

## Status
Accepted

## Context
OpenHouseCAD is a new CAD/BIM engine requiring a modern C++ foundation for geometry kernels, model systems, and long-term maintainability.

## Decision
OpenHouseCAD uses C++23 as the required language standard for the entire repository.

## Reasons

- Modern compile-time programming capabilities.
- Better support for concepts and generic geometry kernels.
- Improved standard library features such as std::expected and std::unreachable.
- Enables a consistent language baseline across all modules.

## Consequences

Positive:
- Cleaner APIs.
- Stronger type safety.
- Better foundation for geometry and kernel development.

Negative:
- Requires modern compilers and toolchains.
- Some older third-party libraries may need compatibility checks.

## Toolchain Requirement

Supported compilers should provide reliable C++23 support.
