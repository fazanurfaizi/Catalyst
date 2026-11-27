# Catalyst

Catalyst is a 2D game engine and playable demo written in modern C++20. The demo is a
Kingdom Rush Frontiers–inspired sandbox with an ECS, sprite batching, 2D lighting,
animation, particles, audio, text rendering, and a small ImGui-based editor.

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)
![OpenGL](https://img.shields.io/badge/OpenGL-3.3-green)
![CMake](https://img.shields.io/badge/CMake-3.16%2B-blue)

## Features

- **ECS** - sparse-set `Registry`, deferred entity destruction, views, and JSON component
  serialization.
- **Rendering** - instanced sprite batch, layered draw list, render pipeline with
  offscreen targets, and an OpenGL 3.3 core renderer.
- **2D lighting** - point, directional, and spot lights with a tiled light culling pass
  and an ambient term.
- **Asset pipeline** - typed asset handles, importers/finalizers, a JSON asset registry,
  `.meta.json` sidecars, dependency graph, and atomic writes.
- **Hot reload** - a polling file watcher reimports changed assets in editor builds.
- **Animation** - sprite-strip clips, an animation library, and an animator component.
- **Text** - TTF font packing into atlases and a batched text renderer.
- **Audio** - miniaudio-backed sound effects and a looping music player.
- **Particles** - CPU particle emitters with configurable curves.
- **Scene tooling** - scene serialization plus an ImGui asset browser, renderer stats,
  and debug panels.
- **Game flow** - a `Game` base class, pluggable `GameState`s, fixed timestep updates,
  and configurable windows.

## Requirements

- CMake 3.16 or newer
- A C++20 compiler (GCC 10+, Clang 12+, or MSVC 19.29+)
- OpenGL 3.3 capable GPU and drivers
- Git and an internet connection for the first configure (dependencies are fetched with
  `FetchContent`)
- Linux: X11/Wayland development headers required by GLFW

Third-party dependencies are fetched and built automatically:

| Dependency | Version | Purpose |
| --- | --- | --- |
| GLFW | 3.4 | Window and input |
| GLAD | 2.0.8 | OpenGL 3.3 core loader |
| GLM | 1.0.1 | Math |
| ImGui | 1.91.5 | Editor and debug UI |
| nlohmann/json | 3.11.3 | JSON serialization |
| stb | master | Image decoding and font packing |
| miniaudio | 0.11.22 | Audio |
| field-reflection | 1.1.1 | Component reflection |

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The executable is written to `build/Catalyst`.

For development with editor features and hot reload:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

### CMake options

| Option | Default | Description |
| --- | --- | --- |
| `GAME_ENABLE_DEBUG` | `OFF` | Default the build type to `Debug` when none is given |
| `GAME_PRODUCTION_BUILD` | `OFF` | Bakes `ASSETS_PATH="./assets/"` and enables install rules |
| `GAME_BUILD_TESTS` | `ON` for top-level builds | Builds the `engine_tests` target |
| `GAME_ENABLE_WARNINGS` | `ON` for top-level builds | Adds `-Wall -Wextra -Wpedantic` (or `/W4`) |
| `GAME_WARNINGS_AS_ERRORS` | `OFF` | Treats warnings as errors |

Example production build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DGAME_PRODUCTION_BUILD=ON
cmake --build build -j
cmake --install build --prefix dist
```

## Run

Run from the repository root so the relative `game.json` and `scene.json` paths resolve:

```bash
./build/Catalyst
```

In non-production builds the asset root is compiled in as an absolute path. A production
build expects an `assets/` directory next to the executable.

### Controls

| Input | Action |
| --- | --- |
| `W` `A` `S` `D` | Move the camera |
| `Q` / `E` | Rotate the camera |
| Mouse wheel | Zoom toward the cursor |
| Left click | Play the hero attack animation and SFX |
| Right click | Move the hero to the clicked world position |
| `F11` | Toggle fullscreen |
| `Esc` | Quit |

The ImGui panels expose renderer stats, resolution presets, asset reimport, game config
and scene saving, and particle-emitter spawning.

## Project layout

```
.
├── CMakeLists.txt          # Top-level project, options, executable target
├── game.json               # Asset registry (handles, paths, settings hashes)
├── scene.json              # Serialized demo scene
├── assets/                 # Fonts, textures, atlases, audio, shaders
├── engine/
│   ├── include/engine/     # Public engine headers
│   ├── src/                # Engine implementation
│   ├── assets/shaders/     # Engine shaders embedded at build time
│   └── cmake/              # Shader embedding helper
├── game/
│   ├── include/game/       # Demo game and states
│   └── src/                # Demo implementation and entry point
├── tests/                  # Engine test suite
└── vendor/                 # FetchContent dependency declarations
```

### Engine modules

| Module | Responsibility |
| --- | --- |
| `core` | `Game` loop, `GameState`, `GameStateManager`, clock, config |
| `platform` | Window creation, display metrics, input plumbing |
| `input` | Key/mouse state and frame transitions |
| `ecs` | Entities, sparse-set storage, views, component registry |
| `components` | Transform, sprite, animator, colliders, lights, particles, audio |
| `systems` | Animation, movement, particles, camera follow, text, audio |
| `graphics` | Renderer, sprite batch, lighting, render targets, text, textures |
| `asset` | Handles, importers, finalizers, cache, registry, watcher, managers |
| `audio` | Audio engine and music player |
| `scene` | Entity naming and scene serialization |
| `editor` | ImGui asset browser and asset editing |

## Asset pipeline

`game.json` is the project asset registry. Each record stores a stable handle, a name, a
path relative to the asset root, a type, and a settings hash:

```json
{
  "assets": [
    {
      "dependencies": [],
      "handle": "eb3f88131e45d21c",
      "name": "main_font",
      "path": "fonts/Roboto-Regular.ttf",
      "settingsHash": 8396310794483152859,
      "type": "Font"
    }
  ],
  "projectName": "Kingdom Rush Frontiers",
  "root": "assets",
  "version": 1
}
```

Import settings live next to the source file in `<file>.meta.json`:

```json
{
  "importer": "catalyst.font",
  "version": 1,
  "settings": {
    "pixelSize": 18.0,
    "atlasWidth": 1024,
    "atlasHeight": 1024,
    "firstCodepoint": 32,
    "charCount": 95
  }
}
```

Texture atlases are described by `.atlas` files that can pull in a directory of images or
generate solid/checkerboard regions:

```json
{
  "sources": [
    { "type": "directory", "path": "sprites" },
    { "type": "solid", "name": "solidWhite", "color": "FFFFFFFF", "size": 16 }
  ]
}
```

Supported asset types and importers:

| Type | Source | Importer |
| --- | --- | --- |
| `Texture2D` | PNG, JPEG | `catalyst.texture2d` |
| `TextureAtlas` | `.atlas` JSON | `catalyst.texture_atlas` |
| `Font` | TTF | `catalyst.font` |
| `Shader` | GLSL | `catalyst.shader` |
| `Sound` | WAV, MP3 | `catalyst.sound` |
| `Animation` | `.anim` | `catalyst.animation` |

In `Debug` and `RelWithDebInfo` builds the engine defines `CATALYST_EDITOR`, which enables
`AssetManagerEditor` and the `AssetWatcher`. Editing a watched file reimports it and bumps
the reload generation, which the demo reacts to by refreshing its atlas, animations, and
text fonts. Other build types use `AssetManagerRuntime` without file watching.

## Tests

The test suite uses a small header-only harness (no external test framework) and covers
asset management, serialization, importers, audio, and game config handling.

```bash
cmake --build build -j
cd build && ctest --output-on-failure
```

Or run the binary directly:

```bash
./build/engine_tests
```

## Adding a game state

`engine::core::Game` owns the window, renderer, assets, audio, and state manager. A game
is a `Game` subclass that installs states; a state overrides the lifecycle hooks it needs.

```cpp
#include "engine/core/Game.hpp"
#include "engine/core/GameState.hpp"

class MyState final : public engine::core::GameState {
public:
  void OnEnter(engine::core::Game &game) override {
    // Acquire assets, build the scene, start music.
  }

  void OnUpdate(engine::core::Game &game, double dt) override {
    // Per-frame logic and input polling.
  }

  void OnRender(engine::core::Game &game) override {
    auto &renderer = game.GetRenderer();
    renderer.BeginFrame(camera);
    renderer.RenderSprites(scene, game.Assets());
    renderer.RenderLights(scene);
    renderer.EndFrame();
  }

  void OnImGui(engine::core::Game &game) override {
    // Optional debug UI.
  }
};

class MyGame final : public engine::core::Game {
public:
  MyGame()
      : engine::core::Game([] {
          engine::platform::WindowConfig cfg;
          cfg.title = "My Game";
          return cfg;
        }()) {}

protected:
  void OnStart() override { SetState(std::make_unique<MyState>()); }
};
```

See `game/include/game/KingdomRushGame.hpp` and
`game/src/game/states/PlayingState.cpp` for a complete example.

## Notes

- Rendering requires an OpenGL 3.3 core context; GPU work runs on the main thread.
- First configure downloads dependencies and therefore needs network access.
- The demo reads `game.json` and `scene.json` from the current working directory.
