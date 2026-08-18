---
name: cpp-style
description: This repo's C++ conventions — Google + Allman braces, naming, comment voice, and Doxygen docstring placement. Use whenever writing, editing, reviewing, or refactoring any C++ (.cpp/.h/.hpp) here, including new functions, classes, or files, and for small snippets or single-function edits. Also use when asked to "clean up", "format", "lint", or "make this more idiomatic". Apply by default without being asked. Covers the rules no linter enforces; clang-format/cppcheck/clang-tidy all run at /check, so do not invoke them yourself.
---

# C++ style guide

A consistent style makes a codebase easier to scan, review, and
maintain. Apply every rule below to all C++ code you write or edit for
this user, not just when asked to "clean up" — inconsistency between old
and new code in the same file is its own kind of mess.

This skill covers the conventions a linter *cannot* express — naming,
comment voice, docstring placement. Formatting and static analysis are
handled by `/check` (see **Verification** at the bottom); don't run those
tools by hand.

## Formatting

Base style is Google (`BasedOnStyle: Google`), except braces always go on
their own line (`BreakBeforeBraces: Allman`) — functions, classes,
namespaces, and `if`/`for`/`while` all open a new line for `{`. The
repo's own `.clang-format` at the root is the single source of truth;
this skill does not carry a copy to drift out of sync with it.

## Naming

| Element | Convention | Example |
|---|---|---|
| Classes, structs, enums | `PascalCase` | `Enemy`, `MoveContext`, `ColorPair` |
| Enum members | `PascalCase`, no `k` prefix | `Wall`, `Floor`, `North`, `FogUnexplored` |
| Methods, free functions | `camelCase` | `moveTowardPlayer`, `colorAttr`, `initColors` |
| Member variables | trailing-underscore `camelCase_` | `attackDamage_`, `chaseTurnsRemaining_` |
| Constants | `kPascalCase` | `kCap` |
| Files | `snake_case` | `goal_map_cache.h` |

This matches the existing codebase, not strict upstream Google
function-naming (which wants `PascalCase` functions) — don't "correct"
existing camelCase methods to PascalCase.

## Comments

Every comment ends in a period, and comment text is lowercase throughout
(including the first word) — this keeps the visual weight of comments
low relative to code. Use comments to explain *why* something is done a
particular way, not to restate what well-named identifiers and
straightforward operations already make obvious. A comment on
self-explanatory code is noise the next reader has to filter out.

## Docstrings

Use `/** */`-style Doxygen block docstrings — **only** on class methods
and constructors. Never add one to a class declaration, struct
declaration, enum declaration, or a free function.

Shape:
- `@brief <description>` line first.
- `@param <name> <description>` — one per parameter, in signature order.
  Omit entirely if the method/constructor takes no parameters.
- `@return <description>` — a real description of what's returned, not
  a bare restatement of the type (avoid e.g. `@return int`). Omit
  entirely for `void`.
- Collapse to a single line (`/** @brief <description>. */`) when
  there's no `@param`/`@return` content. Otherwise use a multi-line
  block: `@brief` line, a blank `*` line, then the `@param`/`@return`
  lines.

```cpp
class Enemy : public Entity
{
 public:
  /**
   * @brief Construct an enemy at a starting position with combat/behavior
   * tuning.
   *
   * @param x Starting column position.
   * @param y Starting row position.
   * @param attackDamage Damage dealt per successful attack.
   */
  Enemy(int x, int y, int attackDamage);

  /**
   * @brief Advance enemy behavior one frame using wall-aware pathfinding.
   *
   * @param ctx Per-frame context (player position, current room, goal-map
   * cache, other enemies, RNG source).
   */
  void moveTowardPlayer(const MoveContext& ctx);

  /** @brief Reduce enemy health by a damage amount. */
  void takeDamage(int damage);
};
```

This matches the existing codebase's docstring style already — no
special-casing needed between old and new code.

## Common mistakes

- Attached braces (`if (x) {`) — this style always breaks before `{`.
- Docstring on a class/struct/enum declaration — docstrings are
  method/constructor-only.
- `@return int` / `@return bool` — restates the signature; describe what
  the value *means* instead.
- Uppercase or non-period-terminated comments (`// Reset the counter`
  instead of `// resets the counter after a room transition.`).
- Renaming existing camelCase methods to PascalCase to match "real"
  Google style — this codebase's naming is camelCase methods by design;
  match it, don't "fix" it.
- Prefixing enum members with `k` (`kWall`, `kNorth`) because they're
  "constants" — enum members are an exception to the Constants row; they
  take bare `PascalCase` by design.
- A `TODO`/`FIXME`/`XXX`/`HACK`/`TBD` comment — pending work goes in a GitHub
  issue, never in the source. CI rejects these outright. If something is worth
  remembering, it is worth an issue; if it is not worth an issue, a comment
  will not save it.

## Verification

Do not shell out to `clang-format`, `cppcheck`, or `clang-tidy` yourself
— `/check` runs all of them, and an extra invocation costs a WSL spawn
(>1s) on Windows for no added coverage:

| When | What runs | Where |
|---|---|---|
| Final step of every change | clang-format `-i`, then cppcheck, clang-tidy, build | `/check` → `scripts/ci-local.sh` |
| On the PR | the same four, on Linux + macOS | `.github/workflows/ci.yml` |

Nothing runs while you write — there is no write-time hook, so code stays
unformatted and unanalysed until `/check`. It applies formatting in place
and reports the rest; fix what it reports. `/pr` will not prepare a PR
over a failing sweep.

Write to this style as you go anyway. `/check` can fix whitespace and
brace placement, but it cannot fix a docstring on the wrong declaration
or a comment that restates the code.
