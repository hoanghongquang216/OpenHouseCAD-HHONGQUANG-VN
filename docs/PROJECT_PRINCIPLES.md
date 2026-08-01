# OpenHouseCAD Product Development Principles

## Vision

OpenHouseCAD is developed as a real CAD product, not a prototype or demonstration project.

## Core Principles

### Simple

- Keep architecture clear and understandable.
- Avoid unnecessary abstractions.
- Every module and class must have a clear responsibility.

### Easy to Use

- APIs should be intuitive.
- Hide internal complexity when possible.
- Prefer workflows that match real CAD user expectations.

### Optimized

- Design for performance from the beginning.
- Avoid unnecessary data copies.
- Keep memory ownership and lifecycle explicit.

### Extensible

The architecture must support future growth:

- Feature modeling
- Assembly
- STEP/IGES exchange
- Boolean operations
- CAM and simulation workflows
- Plugins

## Development Rules

Before adding a module or feature:

1. Define the product purpose.
2. Check architecture impact.
3. Design the API.
4. Implement.
5. Test.
6. Review before expanding.

## CAD Kernel Direction

The kernel should follow production CAD concepts:

- Geometry layer
- Topology layer
- B-Rep model
- Feature history
- Document model

Do not create empty structures only to satisfy file organization.
Every component must contribute to the product architecture.
