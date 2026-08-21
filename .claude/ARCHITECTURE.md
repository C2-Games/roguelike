# Roguelike Architecture

## io/

Both input & output modules are completely stateless and should have no
knowledge of game-specific components (`Player`, `Level`, `Room`, `Enemy`,
`Projectile`). `Game` owns all real game state; each frame it hands `io/`
a plain-data snapshot to render and reads back a plain-data command — `io/`
never holds a reference into `Game`'s object graph across frames.

### ui_manager.cpp/.h

The middleware `Game` talks to for all I/O. Owns the `Renderer` (see
`output/`) and ncurses' lifecycle — `initscr`/`initColors`/`cbreak`/
`noecho`/`keypad`/`curs_set` on construction, `endwin()` on destruction.
`main.cpp` only constructs a `UIManager`, then a `Game`, then calls
`game.run()`.

- `GameCommand pollInput()` — delegates to `input::pollInput()`. Intercepts
  `GameCommand::Resize` itself (calls `getmaxyx` + `Renderer::resizeAll`)
  rather than surfacing it to `Game`, which has no reason to know terminal
  dimensions — it always returns `GameCommand::None` for a resize poll.
- `void render(const RenderState& state)` — pushes the snapshot through the
  owned `Renderer`'s layers and composes.
- `void showStartScreen()` / `void showGameOver()` — the "press SPACE to
  begin" intro and "Game Over!" message, currently raw `printw`/`getch`/
  `std::cout` living in `Game::run()`. Confining them here keeps every
  ncurses call inside `io/`.

### /input/

#### game_commands.h

```cpp
enum class GameCommand
{
  None,
  MoveUp,
  MoveDown,
  MoveLeft,
  MoveRight,
  Attack,
  Quit,
  Resize,  // consumed by UIManager; Game never observes this value
};
```

#### handle_input.cpp/.h

`GameCommand pollInput()` — the only place `getch()`/`KEY_*` constants are
read. Pure ncurses-key → `GameCommand` mapping; no movement, collision,
door, or projectile-spawn logic (that stays in `Game`, driven off the
returned command, exactly as it runs today inside `Game::handleInput()`).
An unmapped key resolves to `GameCommand::None` (the current
`mvprintw(..., "Invalid key")` debug branch is dropped — it reached past
the I/O boundary to print game-facing text).

### /output/

* Completely stateless: layers hold no live references to `Player`/`Level`/
  `Room` — only geometry (from their constructor) and whatever
  `RenderState` they're handed this frame.
* On every frame, `UIManager::render()` pushes a full `RenderState` snapshot
  through `Renderer::compose(state)`, which forwards it to each enabled
  layer's `doRender(const RenderState&)`.
* `renderer.h`, `render_stack.h`, `window_position.h`, and `layers/` move
  here from the current top-level `render/` tree; `RenderStack::doRender()`
  gains a `const RenderState&` parameter.
* `colors.h` splits: the plain `ColorPair` enum stays as shared vocabulary
  (referenced by `Weapon` and other game-side code); `colorAttr()` and
  `initColors()` — the actual ncurses calls — move here.

#### render_state.h

The plain-data snapshot type. No `Player`/`Level`/`Room`/`Enemy` types
appear in it — only value types already used as shared vocabulary
(`Coordinate`, `EntitySymbol`, `ColorPair`).

```cpp
enum class TileVisibility { Visible, Explored, Unseen };

struct TileView
{
  char symbol;
  TileVisibility visibility;
};

struct EntityView
{
  Coordinate position;
  EntitySymbol symbol;
  bool tinted;         // whether to apply tintColor over the symbol this frame
  ColorPair tintColor;  // meaningful only when tinted == true
};

struct ProjectileView
{
  Coordinate position;
  ColorPair color;
};

struct WeaponView
{
  std::string name;
  int damage, speed, range;
  ColorPair color;
};

struct RenderState
{
  std::array<std::array<TileView, Room::HEIGHT>, Room::WIDTH> tiles;
  EntityView player;
  std::vector<EntityView> enemies;          // already alive-filtered
  std::vector<ProjectileView> projectiles;  // already active-filtered
  int playerHealth, playerMaxHealth;
  int roomIndex, roomCount;
  WeaponView weapon;
  double fps;
};
```

Assembled each frame by a new `render_state_builder::build(...)` free
function living in `core/` (it depends on `Player`/`Level`/`Projectile`, so
it belongs on the game-specific side of the boundary, not inside `io/`).
`Game::render()` becomes `uiManager_.render(render_state_builder::build(
player_, level_, projectiles_, currentFps_, playerHitFlashFramesRemaining_
> 0))`.

Per-tile visibility (today's `Room::isVisible`/`isExplored`) is baked into
`TileView` once per tile; `EntityLayer` reads visibility off the same tile
grid instead of holding its own `const Room*`.

**No counters on the render side.** `io/output/` holds zero frame-to-frame
state — not even render-effect timers. The player hit-flash countdown
(today's `EntityLayer::hitFlashFramesRemaining_`, ticked in `doUpdate()`)
moves to a `Game`-owned counter (`playerHitFlashFramesRemaining_`), reset
to `HIT_FLASH_FRAMES` when an enemy's attack lands and decremented once per
frame in `Game::update()`. Each frame, `render_state_builder::build()`
resolves that counter down to the already-decided `EntityView::tinted` /
`tintColor` pair on `RenderState::player` — `io/` only ever renders what
it's told this instant; it never owns or advances a timer. `EntityLayer`
loses its `doUpdate()` override, `triggerPlayerHitFlash()`, and the
`hitFlashFramesRemaining_` member entirely — it becomes a pure
`doRender(const RenderState&)` with no state of its own.