# pomo

A small pomodoro timer for Omarchy: 25 minutes of focus, a 5 minute break, and
a 15 minute break after every fourth session. It follows the Omarchy theme and
language, and sends a desktop notification when a phase ends.

## Shortcuts

| Key   | Action                |
|-------|-----------------------|
| Space | Start or pause        |
| R     | Reset the timer       |
| S     | Skip to next phase    |
| ,     | Settings (durations)  |
| ?     | Show shortcuts        |
| Q     | Quit                  |

## Build

    bin/build      # build/pomo
    bin/test       # offscreen tests
    bin/install    # makepkg -fsi, adds pomo to the app launcher
