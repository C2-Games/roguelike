---
name: cpp-style
description: Enforces a specific C++ code style for C++ projects — Google-based clang-format (with Allman braces), cppcheck/clang-tidy static analysis, existing-codebase naming conventions, and comment/docstring discipline. Use this skill whenever writing, editing, reviewing, or refactoring any C++ code (.cpp/.h/.hpp files) for this user, including new functions, classes, or files — even for small snippets or single-function edits. Also use it when the user asks to "clean up," "format," "lint," or "make this more idiomatic" for any C++ file. Do not wait for the user to explicitly mention style guidelines; apply these rules by default to all C++ output.
---

# C++ style guide

A consistent style makes a codebase easier to scan, review, and
maintain. Apply every rule below to all C++ code you write or edit for
this user, not just when asked to "clean up" — inconsistency between old
and new code in the same file is its own kind of mess.

## Formatting

Base style is Google (`BasedOnStyle: Google` in `.clang-format`), except
braces always go on their own line (`BreakBeforeBraces: Allman`) —
functions, classes, namespaces, and `if`/`for`/`while` all open a new
line for `{`. Run `scripts/format.sh` (or `scripts/format.sh --check`
for a dry-run) against a project: it uses the project's own
`.clang-format` if one exists, otherwise copies in this skill's
`assets/.clang-format`.

## Static analysis

Run `scripts/check.sh [src-dir] [include-dir]`. It runs `cppcheck
--enable=all` unconditionally, then `clang-tidy` (checks: `bugprone-*,
performance-*, readability-*`, with
`-readability-magic-numbers`/`-readability-identifier-length` disabled)
if it can find a `compile_commands.json` under `build/`,
`.build/debug/`, `.build/release/`, or the current directory — clang-tidy
needs that file to know how the project is actually compiled. It uses
the project's own `.clang-tidy` if present, otherwise copies in this
skill's `assets/.clang-tidy`. Fix everything both tools flag rather than
leaving it for the user to catch.

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

## Editor integration

`references/.vscode/settings.json` wires VS Code's C/C++ extension to
format-on-save using the project's own `.clang-format`
(`C_Cpp.clang_format_style: "file"`). It's a reference to drop into a
project's `.vscode/settings.json` when the user is working in VS Code —
not copied automatically.

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

## After writing code

If `clang-format`, `cppcheck`, and `clang-tidy` are available in the
environment, run `scripts/format.sh --check` and `scripts/check.sh`
against any file you've written or edited before considering the task
done. Fix anything they flag.
