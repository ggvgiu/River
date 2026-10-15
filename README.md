# River

A native C++20/OpenGL rewrite based on the commented River Raid 2600 sources in
`RiverRaid2600AssemblerSourceCode`. Open `River.slnx` in Visual Studio and run
the x64 Debug or Release configuration. No downloaded libraries, ROM, assembler,
or runtime asset files are required.

Build requirements: Windows and Visual Studio with the Desktop development with
C++ workload, the v145 MSVC toolset, and a Windows 10/11 SDK. The project uses C++20.

## Playing

The drawing area starts at 640×480. The game keeps a 4:3 picture when resized.
Press a joystick direction or fire to start your turn.

The **Seed** menu shows the current base seed and lets you generate a new random
one, enter one manually, or restore the original `0xA814` (43028). Manual entry
accepts decimal, `0x` hexadecimal, and Atari-style `$` hexadecimal, from 1 to 65535.
Zero is excluded because it locks the original random generator. Applying any
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

The **View** menu toggles **3D valley**; leave it unchecked for the original
top-down presentation. **Camera angle** offers 25° (low), 40° (medium), or 60°
(high), measured down from the horizon. The camera follows the plane laterally
from behind. These settings persist across game resets during this session.
Switching views or angles does not restart or change the simulation, controls,
collision masks, seeds, or difficulty. Raised stepped banks and extruded sprite
pixels provide the 3D look; fog hides the end of the existing 160-row playfield.
The renderer reuses 128 rows of already-generated terrain behind the plane,
including passed objects and their destroyed states. The game already retains
completed course sections; no extra generator calls or random numbers are needed.
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
Two-player mode alternates turns after a crash. Scores, lives, difficulty switches,
and river checkpoints are independent for each player. Three planes initially;
an extra plane every 10,000 points, up to nine. Hold fire for automatic repeat;
only one missile can be in flight.

Fly over fuel depots to refuel, or shoot them for points. Avoid the banks, islands,
ships, helicopters and enemy jets. Shoot a bridge before flying through it.

| Function key | File menu action |
| --- | --- |
| F1 | Reset the selected game |
| F2 | Select one/two players and reset |
| F3 | Pause/resume |
| F4 | Player 1 expert difficulty: direct rather than guided missiles |
| F5 | Player 2 expert difficulty |
| F6 | Sound on/off |
| F7 | Color/black and white |
| F10 | Controls/help |

The game automatically stops advancing while inactive, minimized, resizing, or
using its menus. Opening a dialog also stops gameplay. Returning does not cause
the simulation to catch up for the time spent away.

XInput-compatible USB/Bluetooth controllers are supported now: controller slots
1 and 2 control the corresponding player; use the left stick or D-pad, and A or
the right trigger to fire. Controllers can be connected/disconnected while playing.
Generic HID/DirectInput controllers that do not expose XInput need another input
provider; they are not currently supported.

## Structure

```
River.cpp — Win32 window, messages, repaint timer
    Application — coordinates the modules and runs fixed 60 Hz simulation
        GameInput — keyboard + XInput → two device-neutral PilotInput values
        RiverRaidGame — turns, movement, fuel, shots, collisions, score, lives
            RiverCourse — deterministic river rows and object generation
            SpriteAssets — sprite masks shared with collision detection
        Renderer — Windows/OpenGL context, viewport, frame presentation
            RiverRaidView — draws river, original sprites, effects and HUD
                ValleyView — optional voxel valley and perspective camera
        RenderOptions — presentation-only view and camera settings
        GameAudio — procedural engine, refueling, firing and explosion sounds
MenuActions.cpp — File/Help commands and menu check marks
```

The gameplay module does not include Windows or OpenGL. `Tick()` advances exactly
one NTSC frame; repaint frequency and window size cannot change the game speed.
The renderer receives a const game reference and cannot change the simulation.
There is no graphics or input code in the level generator.

To add another controller backend, implement `InputSource::Read()` and register it
with `GameInput::AddSource()`. Input sources combine strongest axes and OR fire;
the simulation only sees `PilotInput`, never device handles or virtual key codes.
To add another enemy, extend `ObjectKind`, course generation, its sprite/score,
movement/collision policy, and view palette.

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
| `JetStraight`, `JetMove`, sprite A/B data | Generated `SpriteData.h` with interleaved sprite rows |

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

Sprite rows are checked into the project. To regenerate them after changing the
assembly reference, run `tools/ImportSprites.ps1` from PowerShell. The script keeps
the A/B interleaving explicit and requires no assembler.

## Verification

`tests/GameTests.vcxproj` runs gameplay checks without a window or graphics:
deterministic generation, all 48 difficulties, timing/speed, fuel/refueling,
scores, missile hits/guidance, bank/bridge collisions, lives, checkpoints,
reset/game over, and two-player turns.

`tests/RenderTests.vcxproj` draws the actual OpenGL view into a software-rendered
Windows bitmap and checks nonempty frames and OpenGL errors. Run its executable
with the working directory set to `tests`; it writes QA images under `artifacts`.

`tests/SeedDialogTests.vcxproj` exercises the native manual-seed dialog inside
the test process, checking initialization, Apply, Cancel, and zero rejection.

## Attribution

Original River Raid: Carol Shaw, Activision, 1982. Disassembly analysis and comments:
Thomas Jentzsch; expanded analysis: Matt Jernigan (2020 source). The original
commented files are preserved. The derived sprite data retains these attributions.
