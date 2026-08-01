# OpenHouseCAD Testing Strategy

## Goal

Tests are part of the product quality system, not temporary verification scripts.

## Current Foundation

Kernel tests cover:

- Object lifetime basics
- Topology connectivity
- Validation behavior

## Test Layers

### Unit Tests

Validate individual kernel components:

- ObjectId
- ObjectStore
- Handle
- Topology entities

### Topology Tests

Validate B-Rep consistency:

- Vertex connectivity
- HalfEdge relationships
- Edge pairing
- Loop boundaries
- Face boundaries

### Integration Tests

Validate workflows:

- Model creation
- Feature operations
- Import/export

## Rules

- Every new kernel capability requires tests.
- Tests should validate behavior, not implementation details.
- Keep tests readable and maintainable.
