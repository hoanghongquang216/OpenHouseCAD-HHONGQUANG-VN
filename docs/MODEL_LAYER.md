# OpenHouseCAD Model Layer

## Purpose

The Model Layer represents CAD/BIM objects and their lifecycle.

## Architecture

```text
DocumentModel
     |
     v
ModelStore
     |
     v
EntityRegistry
     |
     v
Entity
```

## Entity Structure

```text
Entity
 |
 +-- EntityId
 |
 +-- EntityType
 |
 +-- PropertySet
```

## Transaction Direction

```text
Transaction
     |
     v
TransactionChange
     |
     v
Model Entity
```

## Design Rules

- Model owns domain objects.
- Kernel owns transaction infrastructure.
- Dependencies flow from higher level to lower level.
- Do not introduce circular dependencies.

## Current Foundation

Implemented:

- Entity identity
- Entity registry
- Model storage
- Transaction boundary
