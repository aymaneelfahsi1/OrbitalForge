# OrbitalForge Codex Memory

## Coding Rules

- Do not add inline comments, block comments, doc comments, or explanatory comments inside source files.
- Keep code pure code.
- Use clear names, focused structure, tests, and verification instead of comments.
- Run relevant tests before finishing work.
- Report any test or tooling that could not be run.

## Learning Context

- The user is doing an accelerated 7-day Modern C++ bootcamp in this repository to prepare for C++ interviews.
- The user is under serious time pressure and wants direct, high-effort teaching with explanations, scenario design, code practice, interview framing, and recall checks.
- The user wants to move fast, but progress should still be validated by working code, tests, and concrete evidence.
- Prefer teaching through OrbitalForge project work over isolated theory.

## Current Review Memory

- On 2026-07-23, Day 4 was reviewed against `OrbitalForge_7-Day_Modern_CPP_Bootcamp.md`.
- Release build succeeded with `cmake --build build/release`.
- Release tests passed with `ctest --test-dir build/release --output-on-failure`: 158 of 158 tests passed.
- CLI smoke checks confirmed `simulate`, `compare`, and `benchmark` routes execute.
- `simulate scenarios/two_body.orbit --output /tmp/orbitalforge-day4-review-two-body` produced `trajectory.csv`, `diagnostics.csv`, and `metadata.txt`.
- Missing scenario input returns a useful load error and exit code 3.
- Day 4 is functionally complete enough to move to Day 5.
- Remaining Day 4 hardening item: add true executable-level CLI integration tests to CTest and reduce noisy stdout for long simulations.

## Day 5 Priority

- Day 5 should focus on DSA and interview preparation through real OrbitalForge features.
- Highest-value Day 5 order: benchmark statistics, STL algorithm lab, body-name hash index, top-k heap report, proximity graph BFS/DFS, then complexity writeup.
- Every Day 5 feature should include tests and a short explanation of complexity, iterator/category requirements where relevant, and interview-style tradeoffs.
