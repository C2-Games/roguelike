---
name: cpp-style
description: C++ conventions — Google + Allman braces, naming, commenting, and Doxygen docstring placement. Use whenever writing, editing, reviewing, or refactoring any C++ (.cpp/.h/.hpp) here, including new functions, classes, or files, and for small snippets or single-function edits. Also use when asked to "clean up", "format", "lint", or "make this more idiomatic". Apply by default without being asked. Covers the rules no linter enforces; clang-format/cppcheck/clang-tidy all run at /check, so do not invoke them yourself.
---

# C++ style guide

A consistent style makes a codebase easier to scan, review, and
maintain. Apply every rule below to all C++ code you write or edit for
this user, not just when asked to "clean up" — inconsistency between old
and new code in the same file is its own kind of mess.

This skill covers the conventions a linter *cannot* express — naming,
commenting, docstring placement. Formatting and static analysis are
handled by `/check` (see **Verification** at the bottom); don't run those
tools by hand.

## Formatting

Base style is Google (`BasedOnStyle: Google`), except braces always go on
their own line (`BreakBeforeBraces: Allman`) — functions, classes,
namespaces, and `if`/`for`/`while` all open a new line for `{`. The
repo's own `.clang-format` at the root is the single source of truth;
this skill does not carry a copy to drift out of sync with it.

**Example:**
```cpp
// bad: attached brace.
if (isAlive()) {
  takeDamage(1);
}

// good: allman — the brace always starts its own line.
if (isAlive())
{
  takeDamage(1);
}
```

## Naming

| Element | Convention | Example |
|---|---|---|
| Classes, structs, enums | `PascalCase` | `Enemy`, `FrameState`, `ColorPair` |
| Enum members | `PascalCase`, no `k` prefix | `Wall`, `Floor`, `North`, `FogUnexplored` |
| Methods, free functions | `camelCase` | `moveTowardPlayer`, `colorAttr`, `initColors` |
| Member variables | trailing-underscore `camelCase_` | `attackDamage_`, `chaseTurnsRemaining_` |
| Constants | `ALL_CAPS`, no `k` prefix | `HEIGHT` |
| Files | `snake_case` | `goal_map_cache.h` |
| Locals | named for what they hold or do | `tiers`, `attributes`, `candidates` |

This matches the existing codebase, not strict upstream Google
function-naming (which wants `PascalCase` functions) — don't "correct"
existing camelCase methods to PascalCase.

**Never name a variable `it`, or any suffixed variation of it (`typeIt`,
`tierIt`, `it2`).** An iterator is still a value with a job — name it for
what it holds or what it locates, so the dereference downstream reads as
prose. `auto tiers = catalog_.find(name);` says what `tiers->second` will
be; `auto it = catalog_.find(name);` says nothing at all.


**Example:**
```cpp
// bad: PascalCase method, k-prefixed constant, no trailing underscore.
class enemy_unit
{
 public:
  static const int kMaxHealth = 100;
  void MoveTowardPlayer();

 private:
  int AttackDamage;
};

// good: matches every row of the table above.
class EnemyUnit
{
 public:
  static const int MAX_HEALTH = 100;
  void moveTowardPlayer();

 private:
  int attackDamage_;
};
```


## Comments

Every comment ends in a period, and comment text is lowercase throughout
(including the first word) — this keeps the visual weight of comments
low relative to code. Use comments to explain *why* something is done a
particular way, not to restate what well-named identifiers and
straightforward operations already make obvious. A comment on
self-explanatory code is noise the next reader has to filter out. Do not
add long comments. Do not mention other commits, issues, PRs, or the
current task — describe the code as it stands, not the change that
produced it; that context won't mean anything once the change is old
news.

**Moving or relocating existing code does not grandfather its comments.**
When a task says a function/struct/block is "moved," "ported," or
"unchanged" from its old file, that instruction covers the *logic*, not
the comment formatting. Re-check every comment and docstring you carry
over against this page before you finish — capitalization, trailing
period, and docstring placement (see below) are not exempt just because
the code predates this convention or the task called the move "verbatim."
Fix the comment's style even when its wording is otherwise fine.

Before finishing any edit, re-scan every comment/docstring you touched or
moved and check it against: lowercase first word, trailing period,
explains *why* not *what*, no docstring on anything but a method/
constructor. This is a self-check to run per comment, not just a
reference to consult if something looks off.

**Example:**
```cpp
// bad: restates what the code already says, wrong casing/period.
// Increment health by heal amount
health_ += healAmount;

// good: the code is already clear, so skip the comment entirely.
health_ += healAmount;

// bad: narrates the current change and points at a tracker instead of
// describing the code as it stands.
// this refactor clamps health instead of asserting; see PR #482.
health_ = std::min(health_ + healAmount, maxHealth_);

// good: explains why the code is written this way, with nothing tying it
// to the change that introduced it.
// clamped rather than asserted: overheal from stacked buffs is expected
// in the late game and should just cap out silently.
health_ = std::min(health_ + healAmount, maxHealth_);
```

## Docstrings

Use `/** */`-style Doxygen block docstrings — never `///`. This covers
class methods and constructors (public **or** private) and standalone
free functions alike; visibility and namespace scope don't change
whether a docstring belongs, only the declaration itself does. Never add
one to a class declaration, struct declaration, or an enum declaration.
If a struct/class/enum carrying a
Doxygen block gets relocated (e.g. into a new header during a refactor),
convert its docstring to a plain `//` comment (or drop it if the block was
pure restatement) rather than carrying the `/** @brief */` block over
as-is — this applies regardless of whether the original file predates
this rule.

Shape:
- `@brief <description>` line first.
- `@param <name> <description>` — one per parameter, in signature order.
  Omit entirely if the method/constructor takes no parameters.
- `@return <description>` — a real description of what's returned, not
  a bare restatement of the type (avoid e.g. `@return int`). Omit
  entirely for `void`.
- Collapse to a single line (`/** @brief <description>. */`) only when
  the method takes no parameters *and* returns `void` — i.e. there's
  genuinely no `@param`/`@return` content to add. A method with even one
  parameter, or a non-`void` return, always gets the multi-line block:
  `@brief` line, a blank `*` line, then the `@param`/`@return` lines.
- **DO NOT ADD ADDITIONAL NOTES**. For example adding a note between the `@brief` & the `@return` or `@param`.

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
   * @param frame Per-frame world state (player, current room, live enemies).
   * @param cache Goal-map cache reused across every enemy this frame.
   * @param services RNG source.
   */
  void moveTowardPlayer(const FrameState& frame, GoalMapCache& cache,
                        GameServices& services);

  /**
   * @brief Check whether the player is close enough to trigger a chase.
   *
   * @param frame Per-frame world state, used for the player's position.
   * @return True if the player is within this enemy's chase radius.
   */
  bool isWithinChaseRadius(const FrameState& frame) const;

  /**
   * @brief Report whether the enemy still has health remaining.
   *
   * @return True if the enemy has not yet been reduced to zero health.
   */
  bool isAlive() const;

  /**
   * @brief Reduce enemy health by a damage amount.
   *
   * @param damage Amount of damage to apply.
   */
  void takeDamage(int damage);

  /** @brief Reset the enemy back to its starting position and full health. */
  void resetToSpawn();

 private:
  /**
   * @brief Recompute this frame's AI state from FoV and chase-memory data.
   *
   * @param inFoV Whether the player is currently in this enemy's FoV.
   * @param playerPos The player's current position.
   */
  void transitionState(bool inFoV, Coordinate playerPos);
};

/**
 * @brief Convert a room-file legend character into its tile type.
 *
 * @param ch The character read from the room's ASCII grid.
 * @return The tile type that character represents.
 */
TileType charToRoomTile(char ch);
```

This matches the existing codebase's docstring style already — no
special-casing needed between old and new code, and none between public
and private methods, or between a method and a free function.

## Common mistakes

- Attached braces (`if (x) {`) — this style always breaks before `{`.
- Docstring on a class/struct/enum declaration — docstrings belong on
  the method/constructor/free-function declaration, never the type.
- A `//` comment on a private method or a standalone function where a
  `/** @brief */` docstring belongs — private and free functions are not
  exempt.
- `///`-style docstrings — this codebase only uses `/** */` blocks.
- `@return int` / `@return bool` — restates the signature; describe what
  the value *means* instead.
- Collapsing to a single-line `/** @brief ... */` when the method
  actually takes a parameter or returns non-`void` — the one-liner is
  only for zero-parameter, `void` methods; anything else needs the
  multi-line block.
- Uppercase or non-period-terminated comments (`// Reset the counter`
  instead of `// resets the counter after a room transition.`).
- A comment or docstring that narrates the current change ("this refactor...", "as
  part of...") or points at a commit/issue/PR — describe the code as it
  is, not how or why it changed.
- Renaming existing camelCase methods to PascalCase to match "real"
  Google style — this codebase's naming is camelCase methods by design;
  match it, don't "fix" it.
- Prefixing enum members or constants with `k` (`kWall`, `kNorth`, `kConstant`).
- A `TODO`/`FIXME`/`XXX`/`HACK`/`TBD` comment — pending work goes in a GitHub
  issue, never in the source. If something is worth remembering, it is worth
  an issue; if it is not worth an issue, a comment will not save it.

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