# OpenHouseCAD Multi AI Development Workflow

## Purpose

Define collaboration rules for multiple AI development machines working on one repository.

## Roles

### Core Architect

Responsible for:

- Architecture decisions
- Kernel and model integration
- Core implementation

### Review Engineer

Responsible for:

- Code review
- Tests
- Validation
- Finding regressions

### Build Engineer

Responsible for:

- CMake
- CI/CD
- Documentation
- Developer workflow

## Rules

- One owner per source area.
- Do not modify another role's active files without agreement.
- All changes go through Git history.
- Tests should accompany feature changes.

## Flow

```text
Feature
  |
  v
Implementation
  |
  v
Review
  |
  v
Build Verification
  |
  v
Integration
```
