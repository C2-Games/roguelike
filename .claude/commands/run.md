---
description: Build debug and run the game, then report the logs
---

1. Build debug first (see `/build`) — never run a stale binary.

2. Run it **from the repo root**: the game resolves `assets/` relative to the working directory,
   so running from inside `.build/debug/` fails to load rooms and enemies.

   It is a full-screen ncurses app and will hang waiting on input if run bare, so drive it
   non-interactively and give it a real TTY. `script` takes different arguments on GNU and BSD, so
   detect which one is present — macOS ships the BSD flavour:

   ```bash
   python3 .claude/hooks/toolchain_run.py '
     rm -f game.log error.log
     if script --version 2>/dev/null | grep -qi util-linux; then
       printf "q" | script -qc "./.build/debug/roguelike" /dev/null >/dev/null 2>&1
     else
       printf "q" | script -q /dev/null ./.build/debug/roguelike >/dev/null 2>&1
     fi
     echo "exit=$?"'
   ```

   Change the `printf` payload to send whatever key sequence the test needs; `q` quits immediately.

3. **Report the logs** — this is the output actually worth reading. `Logger::get()` writes both at
   the repo root via the `LOG` / `LOG_ERR` macros:

   ```bash
   python3 .claude/hooks/toolchain_run.py 'tail -40 game.log; echo "--- errors ---"; tail -20 error.log'
   ```

4. If anything is left running, clean it up:

   ```bash
   python3 .claude/hooks/toolchain_run.py "pkill -9 -f '.build/debug/roguelike'; pkill -9 script; echo done"
   ```

Never write `wsl.exe` into a command — it breaks the Mac/Linux developer.
