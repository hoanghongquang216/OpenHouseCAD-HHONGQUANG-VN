# OpenHouseCAD Kernel Architecture

## Goal

The kernel is designed as a production CAD kernel foundation.

It must remain simple to use while supporting future feature modeling, exchange formats, and advanced operations.

## Layering

### Geometry Layer

Responsible for mathematical representation:

- Points
- Curves
- Surfaces
- Geometry evaluation

### Topology Layer

Responsible for connectivity:

- Vertex
- HalfEdge
- Edge
- Loop
- Face
- Shell
- Solid

Topology does not own heavy geometry data.

## Ownership Model

Objects are managed through stable identities and controlled lifetime management.

Preferred direction:

ObjectId -> ObjectStore -> Handle<T>

Avoid uncontrolled ownership graphs.

## B-Rep Direction

The kernel must support:

- Face boundaries
- Edge adjacency
- Vertex connectivity
- Solid validation
- Boolean preparation
- STEP/IGES style exchange

## Implementation Rules

- Do not add empty classes only for structure.
- Keep APIs simple.
- Preserve future extensibility.
- Add tests with every core capability.
