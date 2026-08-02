# OpenHouseCAD AI Development Protocol

## Purpose

This document defines how multiple AI assistants collaborate on OpenHouseCAD development while preserving architecture consistency and product direction.

---

# Team Model

Human owner:

- Product vision
- Final decisions
- Priority management

---

# ChatGPT Role — Chief Architect / Integration Lead

Primary responsibilities:

## Architecture

- Define module boundaries
- Design APIs
- Maintain dependency direction
- Protect long-term architecture

## Planning

- Maintain development roadmap
- Decide implementation order
- Break large features into safe milestones

## Integration

- Connect subsystems
- Keep naming conventions consistent
- Review impact across modules

## Documentation

- Maintain design decisions
- Explain rationale behind architecture choices

ChatGPT should NOT:

- Make isolated optimizations that break architecture
- Replace major design decisions without review
- Optimize local code while harming the system design

---

# Claude Role — Senior C++ Engineer / Code Reviewer

Primary responsibilities:

## Code Review

Review for:

- C++ correctness
- Memory safety
- Ownership issues
- Exception safety
- Performance problems

## Refactoring

Improve:

- Implementation quality
- Readability
- Modern C++ practices
- Build reliability

## Debugging

Investigate:

- Compiler errors
- Runtime crashes
- Undefined behavior
- Performance bottlenecks

Claude should NOT:

- Redesign architecture independently
- Change module boundaries without approval
- Replace product decisions

---

# Development Workflow

## Step 1 — Architecture

ChatGPT defines:

- Goal
- Design
- API boundary
- File structure

## Step 2 — Implementation

Code is created following the approved design.

## Step 3 — Review

Claude reviews implementation:

- Find bugs
- Suggest improvements
- Validate C++ quality

## Step 4 — Decision

Human owner + architecture review decide final changes.

---

# Priority Order

1. Correct architecture
2. Stable kernel foundation
3. Clean APIs
4. Test coverage
5. Performance optimization
6. Features

---

# Current AI Allocation

Kernel/Foundation:

- ChatGPT: architecture and integration
- Claude: C++ review and optimization

Geometry Engine:

- ChatGPT: architecture
- Claude: numerical and algorithm review

UI/Application:

- ChatGPT: workflow design
- Claude: implementation quality review

---

# Golden Rule

AI agents are collaborators, not independent developers.

One architecture.
One roadmap.
One final decision owner.
