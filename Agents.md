# AGENTS.md

# PROJECT AI DEVELOPMENT STANDARD

This file defines how AI agents must work on this repository.

The repository contains a desktop application that currently uses Electron to retrieve and display YouTube live comments/chat as a customizable desktop overlay.

The project should be developed to professional software-engineering standards.

The priorities are:

1. Correctness
2. Reliability
3. Maintainability
4. Security
5. Performance
6. Resource efficiency
7. Simplicity
8. Developer experience

Do not optimize for producing the most code.
Optimize for producing the smallest amount of correct, maintainable code.

---

# 1. PROJECT CONTEXT

The current application:

- is an Electron desktop application
- communicates with the YouTube API
- retrieves live YouTube comments/chat
- displays comments in a desktop overlay
- supports some customization and themes

Known development goals include:

- configurable API request frequency
- improved YouTube API quota efficiency
- configurable overlay transparency
- improved themes
- more customization
- lower CPU usage
- lower memory usage
- faster startup
- smaller application footprint
- cleaner architecture
- stronger testing
- reliable error handling
- professional code quality

A future native implementation may be considered, particularly using C++, but this is NOT an assumption or immediate requirement.

Native migration must be justified by architecture and measurable performance benefits.

---

# 2. SOURCE OF TRUTH

The repository contains project documentation divided by responsibility.

Do not read every documentation file for every task.

Read only the files relevant to the current task.

## Project Memory

`PROJECT_MEMORY.md`

Contains stable, high-value information about:

- project purpose
- goals
- technology choices
- important constraints
- important long-term context

Keep this concise.

---

## Architecture

`docs/ARCHITECTURE.md`

Describes the actual technical architecture.

Read when working with:

- Electron processes
- application structure
- data flow
- state management
- services
- IPC
- APIs
- module relationships
- major refactoring

---

## Features

`docs/FEATURES.md`

Describes:

- implemented features
- partially implemented features
- planned features
- known limitations

Read when implementing or modifying functionality.

---

## API

`docs/API.md`

Describes:

- YouTube API integration
- API endpoints
- request flow
- polling
- quota behavior
- retries
- errors
- rate limiting
- API-related constraints

Read before modifying API or polling logic.

---

## Product Design

`docs/DESIGN.md`

Describes:

- product UX
- overlay behavior
- layout philosophy
- interaction patterns
- visual direction
- user experience decisions

Read when making meaningful UI/UX changes.

---

## Design System

`docs/DESIGN_SYSTEM.md`

Describes:

- colors
- typography
- spacing
- radii
- shadows
- animation
- themes
- design tokens
- reusable visual rules

Read when modifying visual styling or themes.

---

## Component Library

`docs/COMPONENT_LIBRARY.md`

Describes reusable UI components.

Before creating a new component:

1. Check this document.
2. Inspect the relevant existing component.
3. Reuse or extend it when appropriate.

Avoid creating duplicate components.

---

## Performance

`docs/PERFORMANCE.md`

Contains:

- performance measurements
- known bottlenecks
- optimization history
- CPU usage
- memory usage
- startup measurements
- package-size measurements
- rendering performance

Read when optimizing or evaluating native migration.

Performance decisions should be based on measurements where practical.

---

## Testing

`docs/TESTING.md`

Contains the project's testing strategy and test infrastructure.

Read when:

- adding tests
- modifying test infrastructure
- fixing bugs
- adding critical functionality
- changing API or polling behavior

---

## Code Standards

`docs/CODE_STANDARDS.md`

Contains project-specific coding conventions.

Follow these conventions unless they conflict with correctness or established framework standards.

---

## Quality

`docs/QUALITY.md`

Contains:

- quality requirements
- quality gates
- validation requirements
- definition of done

---

## Decisions

`docs/DECISIONS.md`

Contains important architectural and technical decisions and their reasoning.

Read before revisiting an established architectural decision.

Do not reopen a settled decision without new evidence or requirements.

---

## Implementation Plan

`docs/IMPLEMENTATION_PLAN.md`

Contains the high-level development roadmap and priorities.

Read for large or multi-step work.

---

## Tasks

`docs/TASKS.md`

Contains concrete actionable work.

Use this when selecting or continuing development tasks.

---

## Progress

`docs/PROGRESS.md`

Contains meaningful project progress and milestones.

Do not turn this into a detailed development diary.

---

## Current Session

`docs/CURRENT_SESSION.md`

Contains short-term context for substantial or interrupted work.

Use only when useful.

It is not permanent project memory.

Keep it concise.

---

## Changelog

`docs/CHANGELOG.md`

Contains meaningful user-visible and architectural changes.

Do not record every small code modification.

---

# 3. CORE DEVELOPMENT WORKFLOW

For meaningful tasks follow:

INSPECT
→ UNDERSTAND
→ PLAN
→ IMPLEMENT
→ TEST
→ VERIFY
→ DOCUMENT

Do not start coding before understanding the relevant existing implementation.

For trivial changes, use reasonable judgment and avoid unnecessary ceremony.

---

# 4. INSPECT BEFORE MODIFYING

Never assume how the application works.

Inspect the actual code.

Before modifying a meaningful feature:

1. Identify the relevant files.
2. Trace the existing behavior.
3. Check related components/utilities.
4. Check relevant documentation.
5. Check existing tests.
6. Check existing architectural decisions.
7. Identify possible regression risks.

Do not infer architecture solely from filenames.

---

# 5. MINIMAL CHANGE PRINCIPLE

Prefer:

- modifying existing code
- extending existing abstractions
- reusing existing components
- reusing existing utilities
- simplifying existing code
- removing genuinely unnecessary code

Avoid:

- unnecessary rewrites
- duplicate functionality
- duplicate components
- speculative abstractions
- unnecessary dependencies
- unrelated refactoring
- changing working architecture without a reason

Do not rewrite a working system simply because another implementation appears cleaner.

---

# 6. ARCHITECTURE PRINCIPLES

Keep responsibilities separated.

Where appropriate, maintain clear boundaries between:

- UI
- application state
- application logic
- YouTube API layer
- polling/scheduling
- configuration
- persistence
- data transformation
- system/window integration

Do not introduce abstraction layers merely for theoretical cleanliness.

An abstraction should solve a real maintainability, testing, or architectural problem.

---

# 7. ELECTRON PRINCIPLES

Respect Electron's process architecture.

Determine whether work belongs in:

- main process
- renderer
- preload
- worker/thread
- external/native process

Avoid:

- blocking operations
- unnecessary synchronous work
- unnecessary IPC
- excessive renderer updates
- unnecessary background processing
- unnecessary Electron features
- unnecessary dependencies

Do not introduce native code solely because it sounds faster.

---

# 8. YOUTUBE API PRINCIPLES

YouTube API quota efficiency is a major project requirement.

Polling must eventually have:

- centralized configuration
- configurable request interval
- sensible default
- sensible minimum
- validation
- duplicate-loop prevention
- overlapping-request prevention
- error handling
- appropriate retry/backoff behavior

Never scatter polling intervals throughout the codebase.

Normal automated tests must NOT repeatedly call the real YouTube API.

Use mocks, fixtures, or dependency injection for normal tests.

Real API tests, if necessary, must be isolated and explicitly controlled.

Never expose API credentials.

---

# 9. CONFIGURATION PRINCIPLES

User-configurable values must have a clear source of truth.

Potential settings include:

- API request interval
- transparency
- theme
- colors
- font
- font size
- overlay size
- overlay position
- always-on-top behavior
- comment limits

Do not duplicate configuration values across unrelated components.

---

# 10. UI AND DESIGN PRINCIPLES

Use the established design system.

Prefer:

Design Tokens
→ Theme Configuration
→ Reusable Components
→ UI

Avoid hardcoding visual values throughout unrelated components.

Before creating a UI component, check `docs/COMPONENT_LIBRARY.md`.

Before making meaningful visual changes, check:

- `docs/DESIGN.md`
- `docs/DESIGN_SYSTEM.md`

---

# 11. TESTING STANDARD

Use testing tools appropriate to the actual technology stack.

Do not assume a particular framework before inspecting the repository.

For the current JavaScript/TypeScript application, use the existing testing ecosystem when appropriate.

For future C++ code, use an appropriate C++ testing framework such as GoogleTest or Catch2 if suitable.

Testing should include appropriate combinations of:

- unit tests
- integration tests
- mocked API tests
- regression tests
- end-to-end tests

Do not test every trivial line simply to achieve an arbitrary coverage percentage.

Prioritize:

- API logic
- polling
- configuration
- persistence
- state transitions
- data transformation
- error handling
- IPC
- critical user workflows

---

# 12. TESTING EXTERNAL APIs

Normal automated tests must not depend on live YouTube API responses.

Mock or simulate:

- successful responses
- empty responses
- malformed responses
- network failures
- timeouts
- authentication failures
- authorization failures
- rate limits
- quota exhaustion
- unexpected responses

Real API smoke tests may exist separately.

They must:

- be explicitly triggered
- use secure environment variables
- avoid exposing credentials
- avoid unnecessary API quota usage
- never be required for ordinary unit tests

---

# 13. REGRESSION TESTING

When fixing a meaningful bug:

1. Reproduce the problem.
2. Add a regression test when practical.
3. Fix the problem.
4. Verify the regression test passes.

Do not rely solely on manually remembering that a bug was fixed.

---

# 14. ERROR HANDLING

Handle expected failure conditions deliberately.

Consider:

- network errors
- API errors
- invalid API responses
- configuration errors
- persistence failures
- invalid user input
- unexpected state

Do not silently swallow important errors.

Do not expose secrets through errors or logs.

The application should fail gracefully where possible.

---

# 15. TYPE SAFETY AND VALIDATION

Prefer strong typing.

Avoid weak or untyped structures unless justified.

External data must be validated before being trusted by the application.

Do not assume API responses are always valid.

Validate user-configurable values.

---

# 16. SECURITY

Never:

- commit API keys
- hardcode secrets
- expose credentials unnecessarily
- log credentials
- expose privileged functionality to the renderer unnecessarily

Follow least-privilege principles.

Treat external API data and user input as untrusted.

---

# 17. PERFORMANCE

Performance must be measured rather than guessed whenever practical.

Important metrics include:

- RAM usage
- CPU usage
- GPU usage
- startup time
- idle resource usage
- rendering performance
- network requests
- API request frequency
- package size

Before optimizing a suspected bottleneck:

1. Identify it.
2. Measure it when practical.
3. Make a targeted change.
4. Measure again.
5. Keep the optimization if it provides meaningful benefit.

Do not perform speculative micro-optimization.

---

# 18. NATIVE MIGRATION

Do not automatically convert the Electron application to C++ or Java.

First:

1. Understand the current architecture.
2. Establish performance baselines.
3. Identify actual bottlenecks.
4. Optimize the current implementation.
5. Separate architectural boundaries where useful.
6. Evaluate whether native code solves a demonstrated problem.

If native migration becomes justified, prefer an incremental approach.

Potential native boundaries may include:

- API/network engine
- polling engine
- data processing
- persistence
- system/window integration

The UI does not automatically need to be rewritten.

C++ is the preferred native candidate unless the technical requirements indicate otherwise.

---

# 19. CODE QUALITY

Before completing meaningful work, check for:

- unnecessary duplication
- dead code
- unused imports
- temporary debug code
- weak naming
- poor error handling
- unnecessary complexity
- unnecessary dependencies
- type-safety problems
- security problems

Do not leave temporary implementation artifacts behind.

---

# 20. DEPENDENCIES

Do not install a dependency unless it provides a meaningful benefit.

Before adding one:

1. Check whether the functionality already exists.
2. Check whether an existing dependency can provide it.
3. Consider maintenance and bundle-size impact.
4. Consider security implications.
5. Use the smallest appropriate solution.

Do not add libraries simply because they are popular.

---

# 21. DOCUMENTATION MAINTENANCE

Documentation must reflect reality.

Never mark something as implemented unless it has actually been implemented and verified.

Update only relevant documents.

### Update `PROJECT_MEMORY.md`

When stable project context changes.

### Update `ARCHITECTURE.md`

When architecture or data flow changes.

### Update `FEATURES.md`

When features are added, changed, removed, or meaningfully fixed.

### Update `API.md`

When API behavior, polling, quota handling, or network architecture changes.

### Update `DESIGN.md`

When product UX or visual direction changes.

### Update `DESIGN_SYSTEM.md`

When reusable visual rules or tokens change.

### Update `COMPONENT_LIBRARY.md`

When reusable components are added, changed, or removed.

### Update `PERFORMANCE.md`

When meaningful performance measurements or optimizations occur.

### Update `TESTING.md`

When testing strategy or infrastructure changes.

### Update `CODE_STANDARDS.md`

Only when project-specific coding conventions change.

### Update `QUALITY.md`

When quality gates or engineering requirements change.

### Update `DECISIONS.md`

When important technical decisions are made.

### Update `IMPLEMENTATION_PLAN.md`

When priorities or the roadmap materially change.

### Update `TASKS.md`

When tasks are added, completed, reprioritized, or removed.

### Update `PROGRESS.md`

For meaningful milestones.

### Update `CURRENT_SESSION.md`

Only for substantial ongoing or interrupted work.

### Update `CHANGELOG.md`

For meaningful user-visible or architectural changes.

---

# 22. TOKEN EFFICIENCY

The agent should minimize unnecessary context usage.

Do not:

- repeatedly read the entire repository
- repeatedly read every documentation file
- repeatedly summarize the entire project
- reproduce large amounts of source code
- create verbose documentation without value
- re-investigate settled decisions without reason

Prefer:

- targeted file inspection
- targeted documentation
- concise plans
- minimal changes
- existing abstractions
- previously established decisions

The documentation system exists to REDUCE repeated context gathering.

---

# 23. COMMUNICATION

For meaningful tasks, before implementation briefly state:

- what you found
- affected areas/files
- intended approach

After implementation briefly state:

- what changed
- tests/checks performed
- documentation updated
- remaining issues

Do not provide long explanations unless requested.

---

# 24. DEFINITION OF DONE

A meaningful change is complete when applicable items are satisfied:

- requirements understood
- relevant code inspected
- architecture respected
- implementation completed
- edge cases considered
- error handling considered
- tests added or updated where appropriate
- relevant tests pass
- build passes when applicable
- lint/type checks pass when applicable
- no obvious temporary/dead code remains
- documentation updated
- existing functionality preserved
- self-review completed

If something cannot be verified, say so explicitly.

Never claim something passed when it was not tested.

---

# 25. STOP CONDITIONS

Stop and investigate rather than blindly continuing when:

- requirements conflict
- existing architecture is unclear
- tests reveal unexpected failures
- a change could destroy working functionality
- an external API behaves unexpectedly
- credentials or security boundaries are involved
- a large rewrite appears necessary

Do not hide failures.

Report the problem and the evidence available.

---

# 26. GENERAL RULE

Act like a careful professional engineer working in an existing production codebase.

Do not behave like a code generator.

Prefer:

UNDERSTANDING
→ SIMPLE DESIGN
→ SMALL CHANGES
→ TESTING
→ VERIFICATION
→ DOCUMENTATION

The goal is:

HIGH SIGNAL
LOW TOKEN USAGE
CORRECT SOFTWARE
MAINTAINABLE CODE
MEASURABLE PERFORMANCE
STRONG TEST COVERAGE
ACCURATE PROJECT MEMORY