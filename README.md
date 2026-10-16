# River

A native C++20/OpenGL rewrite based on the commented River Raid 2600 sources in
`RiverRaid2600AssemblerSourceCode`. Open `River.slnx` in Visual Studio and run
the x64 Debug or Release configuration. No downloaded libraries, ROM, assembler,
or runtime asset files are required.

Build requirements: Windows and Visual Studio with the Desktop development with
C++ workload, the v145 MSVC toolset, and a Windows 10/11 SDK. The project uses C++20.

## Playing

### Download a Windows build

Open the repository's [Actions page](https://github.com/ggvgiu/River/actions/workflows/windows.yml),
select a successful **Windows build** run, then download **River-windows-x64**
from **Artifacts**. Sign in to GitHub to download, extract the ZIP, and run
`River.exe` on Windows 10/11 x64. The Release executable includes the C++ runtime;
no Visual Studio, ROM, or external asset installation is needed.

GitHub Actions builds and tests pushes to `main` and pull requests using a hosted
Windows/Visual Studio 2026 runner. You can also use **Run workflow** on that page.
CI runs the gameplay and offscreen rendering tests. The native dialog tests
compile in CI but execute only locally, where an interactive desktop is available.
Build artifacts are retained for 30 days. They are development downloads; a
GitHub Release can provide a permanent download later.

To run the same build/tests/package locally, use `tools/BuildWindows.ps1` in
PowerShell. If MSBuild is installed outside the usual location, pass
`-MSBuildPath 'path/to/MSBuild.exe'`. The playable package is in
`dist/windows-x64/`; generated packages are ignored by Git.

### Controls

The drawing area starts at 640×480. The game keeps a 4:3 picture when resized.
Press a joystick direction or fire to start your turn.

The **Seed** menu shows the current base seed and lets you generate a new random
one, enter one manually, or restore the original `0xA814` (43028). Manual entry
accepts decimal, `0x` hexadecimal, and Atari-style `$` hexadecimal, from 0 to 65535.
Zero is allowed; the Seed menu warns that it generates no course variations
because the original random generator stays at zero. Applying any
seed restarts both players; ordinary reset and player selection keep that seed.
The same seed reproduces the same initial course. Seeds are kept for the current
application session, rather than saved to disk.

The **Cheats** menu offers **Infinite lives** (F8) and **Invincibility** (F9).
Infinite lives keeps the current life count when crashing; the explosion, restart,
and alternating turns still happen. Invincibility prevents crashes from banks,
islands, enemies, bridges, and empty fuel; fuel still drains and refueling works.
Both toggles apply to both players, take effect without restarting, and remain
selected across resets, seed changes, and player selection for this session.
They start disabled each time the application is launched. Turning a cheat on
does not undo a crash already underway or revive a finished game; use Reset then.

The game starts in **3D valley** at **60°**. **F5** or the **View** menu toggles it; leave it unchecked for the original
top-down presentation. **Camera angle** offers 25° (low), 40° (medium), or 60°
(high), measured down from the horizon. The camera follows the plane laterally
from behind. These settings persist across game resets during this session.
Switching views or angles does not restart or change the simulation, controls,
collision masks, seeds, or difficulty. Connected, world-anchored terrain surfaces
and solid models provide the 3D look; fog hides the end of the existing 160-row playfield.
Houses and trees have separate ground supports and foundations that fill slopes
beneath them. Terrain geometry lives in `src/rendering/TerrainSurface.cpp`, separate from gameplay.
Water has animated Perlin color ripples, and layered Perlin relief shapes the
banks, islands and surrounding hills. Banks and islands use the same cached
relief and shading, fading to their original
shoreline elevations. `src/rendering/visuals/VisualNoise.h` hashes the selected base seed
without advancing the gameplay random generator. Noise is anchored in world
coordinates, so the same seed reproduces the scenery. Water stays flat beneath
boats; its animation pauses with the game. These effects apply only to 3D mode.
The renderer caches terrain in a bounded ring of world rows and submits triangle
arrays in batches. Scrolling reuses those rows; changes to the seed, river rows,
or color mode invalidate the affected entries. Ripple colors update at 10 Hz
while the scene continues scrolling at the normal frame rate. Run RenderTests
with `--benchmark` to measure stationary and scrolling software OpenGL frames.
Terrain batches outside the camera frustum are rejected before submission.
Fly-bys reuse river rows and a bounded cache of world height samples, checked
against the seed and current course geometry. The benchmark also reports the
cost of helicopter terrain sampling.
**View → Seed-based colors** starts enabled. The default seed keeps the Classic
palette. Other seeds select one of 15 themes: Lava, Amethyst, Desert, Glacier,
Marsh, Obsidian, Copper, Emerald, Rose, Midnight, Alien, Autumn, Chalk, Crimson,
or Golden. Each has 64 deterministic hue/brightness variations, giving **961
palettes** including Classic. These color the river, terrain, islands, and fog
in both views. Uncheck it to use Classic
colors with any seed. The setting survives resets during the session and affects
neither course generation nor gameplay. Theme definitions are in
`src/rendering/visuals/EnvironmentPalette.h`; selecting a theme consumes no random numbers.

The renderer reuses 128 rows of already-generated terrain behind the plane,
including passed objects and their destroyed states. The game already retains
completed course sections; no extra generator calls or random numbers are needed.
Passed enemies keep moving, wrapping or bouncing at banks throughout this retained
region, using their existing movement rules and the same per-frame random step.
The HUD and pause/ready/game-over overlays remain in screen space in both views.

| Action | Player 1 | Player 2 |
| --- | --- | --- |
| Steer left/right | Left/right arrows | A/D |
| Accelerate/brake | Up/down arrows | W/S |
| Fire | Space | F |

Up and down change the scrolling speed; they do not move the aircraft vertically.
The HUD level counts a pair of segments: river + mountains is level 1, the next
river + mountains is level 2, and so on. Each player keeps their own progress;
crashing retains their segment checkpoint, while resetting starts at level 1.
This counter continues upward even after the original difficulty stages reach
their cap and alternate between 47 and 48. Difficulty and generation are unchanged.
The supplied assembly enables enemy jets from internal stage 3, which is the
river half of displayed level 2. Stages 1 and 2 generate only ships/helicopters as
enemies. The exact first jet position depends on the seed; the next segment can
already be visible ahead before the player's own level counter advances.
Two-player mode alternates turns after a crash. Scores, lives, difficulty switches,
and river checkpoints are independent for each player. Three planes initially;
an extra plane every 10,000 points, up to nine. Hold fire for automatic repeat;
only one missile can be in flight.

Fly over fuel depots to refuel, or shoot them for points. Avoid the banks, islands,
ships, helicopters and enemy jets. Shoot a bridge before flying through it.

| Function key | Menu action |
| --- | --- |
| F1 | Reset the selected game |
| F2 | Select one/two players and reset |
| F3 | Pause/resume |
| F4 | Generate a new random seed and restart |
| F5 | Toggle 3D valley / top-down view |
| F10 | Controls/help |

Expert missile difficulty for both players, sound, and black-and-white display
are available in **Options**. F4 now generates a seed; the former F6/F7 bindings
are removed.

The game automatically stops advancing while inactive, minimized, resizing, or
using its menus. Opening a dialog also stops gameplay. Returning does not cause
the simulation to catch up for the time spent away.

XInput-compatible USB/Bluetooth controllers are supported now: controller slots
1 and 2 control the corresponding player; use the left stick or D-pad, and A or
the right trigger to fire. Controllers can be connected/disconnected while playing.
Generic HID/DirectInput controllers that do not expose XInput need another input
provider; they are not currently supported.

## Structure

Source files are grouped on disk as well as in Visual Studio:

```text
src/
    app/        Window entry point, application coordination, menus, seed dialog
    game/       Course generation, simulation, collisions, scoring rules
    input/      Keyboard/controller input and the device-neutral interface
    audio/      Procedural game sounds
    platform/   Shared Windows headers and SDK targeting
    rendering/  OpenGL context, views, terrain and presentation settings
        visuals/        Model library, drawing helpers, noise and palettes
            models/     Individual editable asset definitions
resources/      Menu/dialog resource script, resource IDs and icons
tests/          Gameplay, rendering and native dialog checks
tools/          Collision-mask import script
RiverRaid2600AssemblerSourceCode/   Original reference material
```

Open `River.slnx` at the repository root as before. Source includes use paths
from `src/`, such as `game/RiverRaidGame.h`; application and test projects share
that include root. Resource builds also include `resources/`. Executables still
build to the existing `x64/Debug` and `x64/Release` locations.

```
src/app/River.cpp — Win32 window, messages, repaint timer
    Application — coordinates the modules and runs fixed 60 Hz simulation
        GameInput — keyboard + XInput → two device-neutral PilotInput values
        RiverRaidGame — turns, movement, fuel, shots, collisions, score, lives
            RiverCourse — deterministic river rows and object generation
            CollisionShapes / CollisionData — fixed gameplay collision masks
            GameRules — scoring rules, independent of artwork
        Renderer — Windows/OpenGL context, viewport, frame presentation
            RiverRaidView — draws river, original sprites, effects and HUD
                ValleyView — optional voxel valley and perspective camera
                src/rendering/visuals/AssetLibrary — swappable visual models
                src/rendering/visuals/models — one editable definition per asset
                src/rendering/visuals/AssetDrawing — OpenGL pixels, boxes, and mesh triangles
        RenderOptions — presentation-only view and camera settings
        GameAudio — procedural engine, refueling, firing and explosion sounds
src/app/MenuActions.cpp — menu commands and check marks
```

The gameplay module does not include Windows or OpenGL. `Tick()` advances exactly
one NTSC frame; repaint frequency and window size cannot change the game speed.
The renderer receives a const game reference and cannot change the simulation.
There is no graphics or input code in the level generator.

To add another controller backend, implement `InputSource::Read()` and register it
with `GameInput::AddSource()`. Input sources combine strongest axes and OR fire;
the simulation only sees `PilotInput`, never device handles or virtual key codes.
To add another enemy, extend `ObjectKind`, course generation, its collision/score
and movement policy, then register its independent model in `AssetLibrary`.

## Editing visual assets

Gameplay never reads `src/rendering/visuals/`. Neither view includes collision masks. The two
systems initially have matching silhouettes, but their data is deliberately
independent: editing a visual model cannot change collision checks or scores.

| Asset | Definition |
| --- | --- |
| Player plane (straight/banking, both players) | `src/rendering/visuals/models/PlayerPlane.cpp` |
| Ship | `src/rendering/visuals/models/Ship.cpp` |
| Helicopter (two animation frames) | `src/rendering/visuals/models/Helicopter.cpp` |
| Enemy plane | `src/rendering/visuals/models/EnemyPlane.cpp` |
| Fuel depot | `src/rendering/visuals/models/FuelDepot.cpp` |
| House | `src/rendering/visuals/models/House.cpp` |
| Bridge (including its 3D supports) | `src/rendering/visuals/models/Bridge.cpp` |

Each definition returns an `AssetModel` containing animation frames, animation
timing, altitude/ground placement, mirroring, and explosion size. A frame contains
three separate collections: colored pixel runs for 2D, boxes for voxel models,
and optional colored triangles for custom 3D geometry. No model-file loader is
implemented yet; triangles allow a future loader without coupling it to gameplay.

`MakePixelFrame()` is a convenience for keeping the initial retro look: it builds
pixel runs and boxes from that model's **own** bitmap and palette. To change just
its 3D look, edit `frame.boxes` / `frame.triangles` after that call, or create an
`AssetFrame` directly. Its 2D artwork can stay the same. To change both views,
edit the model's bitmap/palette or supply different pixel runs as well.

Model coordinates use the object's original bottom-left anchor: X runs right,
forward runs upstream, and altitude runs upward. Boxes use local bottom/top
heights; triangles specify X/altitude/forward for each vertex. Frame width and
length describe the visual extent, used for mirroring and visibility, **not**
hitboxes. Larger artwork does not automatically make an object easier to hit.
Shadow footprints currently come from the visual pixel runs.

`RiverRaidView` owns an `AssetLibrary`, shared by the 2D and 3D drawing paths.
Replace a model in code with `view.Assets().SetObject(kind, replacement)` or
`view.Assets().SetPlayer(banking, playerIndex, replacement)`. The camera and terrain
stay in `ValleyView`; HUD and screen-space overlays stay in `RiverRaidView`.
`AssetDrawing` handles OpenGL submission and contains no gameplay decisions.

Collision masks remain in `src/game/CollisionData.h`, selected by `src/game/CollisionShapes.cpp`;
scores remain in `src/game/GameRules.h`. Only edit those when intentionally changing rules.

## Source mapping and fidelity

The supplied assembly is the reference, especially the 2020 Jernigan expansion.

| Assembly | Native equivalent |
| --- | --- |
| `NextRandom16`, seeds `$14/$A8` | `CourseRandom::Next()`, seed `0xA814` |
| `.nextBlock`, `bankFlags`, `newBankWidth` | `RiverCourse` 16×64-line blocks, island state machine |
| `.newEnemy`, `.newFuel`, `.newHouse`, `EnemyIdTab` | Two objects/block, original difficulty-dependent type choices |
| `speedX`, `triggerX`, `speedY`, `triggerY` | `MoveJet()` and integer accumulators in `Tick()` |
| `ScoreTab` | `ObjectScore()`: ship 30, helicopter 60, jet 100, fuel 80, bridge 500 |
| `fuelHi/fuelLo` | 16-bit fuel: −32/frame, +256/frame while touching fuel |
| `MISSILE_SPEED`, difficulty switches | Six lines/frame, one missile, guided/direct modes |
| `SwapPlayers`, saved random state | Alternating turns and per-player level/seed checkpoints |
| `JetStraight`, `JetMove`, sprite A/B data | Generated `src/game/CollisionData.h`; independently editable visual definitions |

This is a playable native adaptation, **not a cycle-accurate translation**. Bank
slopes use interpolated outlines instead of the TIA's pointer-offset scanline
patterns. The seed and feedback function are original, but complete maps/object
positions are not byte-for-byte identical to an Atari run: object placement is
expressed in world coordinates and generation is done ahead of the viewport.
Enemy patrol start timing, intro/death delays, and checkpoint transition timing
are approximations. The bridge uses a readable 13-row silhouette from its original
rail masks rather than overlapping ROM pointers. Sounds are newly synthesized;
they do not emulate the TIA sound registers. Colors, shadows, explosion effects,
and HUD typography are presentation changes. Pixel-mask software collisions replace
the original hardware collision registers.

Collision rows are checked into the project. To regenerate them after changing
the assembly reference, run `tools/ImportSprites.ps1` from PowerShell. It updates
**collision data only**, keeps A/B interleaving explicit, and requires no assembler.
Visual models are edited separately and are never overwritten by that script.

## Verification

`tests/GameTests.vcxproj` runs gameplay checks without a window or graphics:
deterministic generation, all 48 difficulties, timing/speed, fuel/refueling,
scores, missile hits/guidance, bank/bridge collisions, lives, checkpoints,
reset/game over, and two-player turns.

`tests/RenderTests.vcxproj` draws the actual OpenGL view into a software-rendered
Windows bitmap and checks nonempty frames and OpenGL errors. Run its executable
with the working directory set to `tests`; it writes QA images under `artifacts`.

`tests/SeedDialogTests.vcxproj` exercises the native manual-seed dialog inside
the test process, checking initialization, Apply, Cancel, zero acceptance, and invalid entry rejection.

## Attribution

Original River Raid: Carol Shaw, Activision, 1982. Disassembly analysis and comments:
Thomas Jentzsch; expanded analysis: Matt Jernigan (2020 source). The original
commented files are preserved. The derived sprite data retains these attributions.
