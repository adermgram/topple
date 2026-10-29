# Topple

A physics puzzle in the Angry Birds mould. Pull a ball back in a slingshot, let go, and knock down every green target before you run out of balls.

Everything is C++17 with [raylib](https://www.raylib.com/) for drawing and [Box2D](https://box2d.org/) for physics. The sound is generated in code, so there are no audio files.

![Main menu](docs/images/menu.png)
![A round in progress](docs/images/gameplay.png)

## Play

Grab the build for your system from the [releases page](../../releases), unpack it and run `topple`.

- Windows may warn about an unknown publisher, since the program isn't signed. Choose *More info*, then *Run anyway*.
- On macOS, right-click it and choose *Open* the first time, or run `xattr -d com.apple.quarantine topple` once.

Drag the ball back and let go. The dotted line shows the flight. Wood, stone and glass all behave differently: stone is heavy and never breaks, glass cracks and then shatters. Win with balls to spare for up to three stars. Progress is saved in `progress.txt` next to the program.

| | |
|---|---|
| Mouse | Aim, fire, split |
| Esc | Pause, or back out of a menu |
| M | Mute |

## Balls

Every level hands you its own sequence of balls. They are shown at the top right in the order you fire them, with the next one ringed.

| Ball | What it does |
|---|---|
| Standard (red) | The everyday ball. |
| Heavy (dark steel) | Bigger and more than twice as heavy. It is what topples stone and big structures, so levels only give you a few. |
| Splitter (yellow) | Flies like a standard ball. Click while it is in the air and it breaks into three that fan out, which is how you reach targets that are spread apart. |

## Build

You need Git, CMake 3.15 or newer and a C++17 compiler. raylib and Box2D are downloaded on the first configure, so there are no submodules to fetch.

```
git clone https://github.com/adermgram/topple.git
cd topple
cmake --preset default
cmake --build build
cmake --build build --target run
```

### Windows

Pick the preset that matches your compiler. Don't use `default` on Windows: CMake may choose NMake, which only works inside a Visual Studio developer prompt.

| Compiler | Configure | Build |
|---|---|---|
| MinGW-w64 (g++) and [Ninja](https://ninja-build.org/) | `cmake --preset mingw` | `cmake --build build` |
| MinGW-w64 with `mingw32-make` | `cmake --preset mingw-make` | `cmake --build build` |
| Visual Studio 2022 (C++ workload) | `cmake --preset msvc` | `cmake --build build --config Release` |

With Visual Studio the program ends up in `build\Release\`.

### Linux

You also need the X11 and OpenGL headers:

```
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

## How it works

The code is split by job: `app`, `scenes`, `game`, `physics`, `audio`, `ui`. A few choices shape it:

- **Physics is isolated.** Only `physics/` knows about Box2D and metres. The rest of the game uses pixels, y down, like the screen. The simulation runs on a fixed 60 Hz step.
- **Levels are data.** A level is a name, a list of balls, and some blocks and targets. `game/levels.cpp` builds them with a small helper that stacks pieces, so a new level is one short function.
- **Materials and balls are tables.** What a block or ball is made of (weight, colour, whether it breaks) lives in one table each, so adding one is a table entry and a case.
- **The level reports, the scene decides.** `Level::Update` returns what happened (targets destroyed, glass broken, the hardest hit). `Feedback` turns that into sound and screen shake, and `PlayScene` handles the rules.

Releases are built by the workflow in `.github/workflows`; see [docs/releasing.md](docs/releasing.md).

## License

MIT, see [LICENSE](LICENSE). raylib, Box2D and the Poppins font keep their own licenses, which are included with every release.

Thanks to Ramon Santamaria and the raylib contributors, Erin Catto for Box2D, and Indian Type Foundry for Poppins.
