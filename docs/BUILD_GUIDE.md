# OpenHouseCAD Build Guide

## Overview

This document defines the build workflow for OpenHouseCAD.

## Goals

- Keep modules independently buildable.
- Verify kernel and model layers before adding application features.
- Provide a common workflow for multiple AI development machines.

## Build Flow

```text
Source
  |
  v
CMake Configure
  |
  v
Build Targets
  |
  v
Run Tests
```

## Module Direction

```text
Kernel
  |
  v
Model
  |
  v
Geometry
  |
  v
Application
```

## Rules

- Do not bypass module boundaries.
- Keep C++ standard settings consistent.
- Add tests together with new modules.
