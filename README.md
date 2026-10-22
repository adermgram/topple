# Topple

A physics puzzle game in the style of Angry Birds, written in C++17 with [raylib](https://www.raylib.com/) and [Box2D](https://box2d.org/).

Drag the ball back in the slingshot, release, and knock down every target before you run out of shots.

## How to play

| Action | Control |
|---|---|
| Aim and fire | Press the ball with the mouse, drag back, release |
| Cancel a shot | Release the ball close to the sling |
| Back to the menu | `Esc` or the Menu button |

- You have 5 shots per round.
- The dotted line previews the flight path. It ignores obstacles, so it shows where the ball goes if nothing is in the way.
- Hard hits damage a target and destroy it. Gentle bumps do nothing.
- You win when every target is gone. You lose when you're out of shots and everything has stopped moving.

## Build and run

Requires CMake 3.15+ and a C++17 compiler. raylib and Box2D are downloaded by CMake on the first configure, so the first build needs an internet connection.

```
cmake --preset mingw        # or: cmake --preset default
cmake --build build -j 8
cmake --build build --target run
```

Presets: `default`, `debug`, and `mingw` (MinGW Makefiles generator, used on Windows with a MinGW-w64 toolchain; the executable is linked statically so it needs no DLLs).

On Linux you need the X11 and OpenGL development packages:

```
sudo apt-get install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

The game loads fonts relative to the executable (`resources/` is copied next to it at build time), so it runs from any folder.

## Project layout

```
src/
  app/      App, Scene interface, SceneManager, AppContext, Pointer (mouse state)
  physics/  Box2D wrapper: pixels <-> metres, fixed time step, collision impacts
  game/     Level, Block, Ball, Target, Slingshot, Effects (particles)
  ui/       Theme (palette, fonts), Button, ResultBanner
  scenes/   MenuScene, PlayScene (the rules: shots, win and lose)
resources/
  fonts/    Poppins
```

Design notes:

- **Scenes.** The `SceneManager` holds the active scene and applies replacements at the start of the next frame, so a scene can safely ask to be replaced from inside its own update.
- **Physics is isolated.** Only `physics/` knows about Box2D units. Everything else works in pixels with y pointing down, like the screen. The simulation runs on a fixed 60 Hz step, so it behaves the same at any frame rate.
- **Level vs. rules.** `game::Level` owns the physics world and everything in it. `PlayScene` owns the rules and the UI. Adding a level means adding content to the `Level`, not touching the rules.
- **Input.** Game objects such as the `Slingshot` receive a `Pointer` struct instead of polling raylib, and report what happened (a `Launch`) instead of creating bodies themselves.

## Credits

- [raylib](https://www.raylib.com/) by Ramon Santamaria and contributors (zlib license): windowing, drawing, input.
- [Box2D](https://box2d.org/) 2.4.1 by Erin Catto (MIT license): 2D physics.
- [Poppins](https://fonts.google.com/specimen/Poppins) by Indian Type Foundry (SIL Open Font License; see `resources/fonts/OFL.txt`): UI font.
