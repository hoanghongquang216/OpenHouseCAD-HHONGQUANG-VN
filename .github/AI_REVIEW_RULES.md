# OpenHouseCAD AI Review Rules

## Purpose

Define rules for AI-assisted code review and collaboration in OpenHouseCAD.

The goal is to use AI as engineering support while preserving one consistent architecture.

---

# AI Roles

## ChatGPT — Architecture Owner

Responsibilities:

- Maintain system architecture direction
- Review module boundaries
- Design APIs
- Protect long-term maintainability
- Coordinate integration decisions

ChatGPT decisions focus on:

- Why we build something
- Where it belongs
- How modules communicate

---

## Claude — Senior C++ Reviewer

Responsibilities:

- Review implementation quality
- Find bugs
- Analyze C++ correctness
- Suggest refactoring
- Check performance
- Check memory ownership

Claude focuses on:

- Is the code safe?
- Is the code clean?
- Is the implementation robust?

---

# Claude Review Rules

Claude should:

1. Review before large merges
2. Identify critical problems first
3. Explain technical reasons
4. Suggest minimal safe changes
5. Respect existing architecture

Claude should NOT:

- Redesign architecture independently
- Change public APIs without approval
- Remove abstractions without discussion
- Optimize prematurely

---

# Pull Request Review Checklist

## Architecture

- Does this change respect module boundaries?
- Are dependencies flowing in the correct direction?
- Does it introduce unnecessary coupling?

## C++ Quality

- Correct ownership?
- Correct lifetime management?
- Proper move/copy behavior?
- Exception safety?

## Performance

- Avoid unnecessary allocations
- Avoid unnecessary copies
- Consider scalability

## Testing

- Is there a regression test?
- Are edge cases covered?

## Maintainability

- Are names clear?
- Is the API easy to use?
- Is the design extensible?

---

# Change Classification

## Safe Changes

Examples:

- Add tests
- Improve comments
- Refactor internal implementation
- Fix bugs without API changes

## Review Required

Examples:

- Change public API
- Change module dependency
- Replace architecture pattern
- Remove existing abstractions

## Owner Approval Required

Examples:

- Change core architecture
- Change product direction
- Replace major subsystems

---

# Branch Policy

Recommended branches:

- main: stable product
- codex/*: architecture implementation
- claude/*: review fixes or experimental improvements

AI generated changes must go through review before merge.

---

# Golden Rule

AI helps build the system.

AI does not own the system.

Architecture consistency is more important than local code improvement.
