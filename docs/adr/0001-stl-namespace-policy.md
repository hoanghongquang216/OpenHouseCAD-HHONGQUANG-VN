# ADR 0001: STL Namespace Policy

## Status
Accepted

## Context
OpenHouseCAD is a large C++ codebase. Direct usage of the STL across modules can create inconsistent conventions and make future platform abstraction harder.

## Decision
New public APIs should avoid exposing raw STL types when a project abstraction provides clear value.

The project does not blindly wrap every STL type. Wrappers are introduced only for:

- ownership boundaries;
- public ABI boundaries;
- domain-specific containers;
- serialization boundaries.

Internal implementation code may use STL directly.

## Consequences

Positive:
- clearer module ownership;
- controlled public interfaces;
- easier future refactoring.

Negative:
- some duplication may exist where wrappers are justified.
- developers need to decide boundary ownership carefully.
