# Visual models

Start in `models/`: each asset has one factory function and its own artwork.
These files are rendering data; they are never used by gameplay.

Houses and trees use solid 3D volumes. `House.cpp` defines
walls, windows, a door, a stepped pitched roof, a trunk and layered foliage. Its
original top-down pixels stay separate. `StandPixelRows()` is also available for
assets that should use an upright extruded sprite rather than custom geometry.

`PlayerPlane.cpp` likewise keeps the original 2D sprite while defining a solid
3D fuselage, stepped nose, swept wings, cockpit canopy, tail and exhaust. Its
banking variant offsets the two wings vertically; direction mirroring switches
the bank. Both player colors use the same geometry and original collision masks.

Enemy ships have hulls, raised cabins and masts. Helicopters have a fuselage,
cockpit, tail boom, skids and animated rotors. Enemy jets have a fuselage, wings,
canopy, tail and exhaust, oriented across the river. Each remains in its own
model file. `FuelDepot.cpp` draws a small aircraft carrier in 3D, with a flight
deck, runway markings, island and radar mast. It is still a fuel depot for
gameplay: fly over it to refuel or shoot it for the original points. All 2D
artwork and animation cadence, collision masks and spawn rules stay unchanged.

- `AssetModel.h`: frames, pixels, boxes, triangles, animation and placement.
- `AssetLibrary.cpp`: registers the built-in models; `SetObject` / `SetPlayer`
  replace an asset without changing either view's scene loop.
- `AssetDrawing.cpp`: OpenGL submission for 2D, 3D, mirroring, and shadows.
- `models/BuiltinModels.h`: declarations for the individual model factories.

For example, to give a ship a new 3D body while retaining its current 2D artwork,
edit `MakeShip()` after `MakePixelFrame()`:

```cpp
auto& frame = model.frames[0];
frame.boxes.clear();
frame.boxes.push_back({0, 6, 16, 4, 0, 2, {0.46f, 0.73f, 0.72f}});
frame.boxes.push_back({5, 10, 6, 3, 0, 5, {0.93f, 0.53f, 0.25f}});
```

A box is `{x, forward, width, length, bottom, top, color}`. Heights are relative
to `model.altitude`; `groundMounted` assets use `frame.groundAnchors` for terrain
placement. Each anchor specifies `{x, forward, width, length, foundationColor}`;
assign its index to each box's optional eighth field, `groundAnchor`. The renderer
raises that group to the highest point under its footprint and fills the slope
below it with foundations. House and tree groups can therefore sit at different
heights. Keep a model's
width/length large enough to contain its geometry so culling and mirroring work.
The values here are all visual: collision masks remain in `../CollisionData.h`.

For a smoother shape, add triangles to `frame.triangles` instead of boxes. Each
triangle has three `{x, altitude, forward}` vertices and one RGB color. Editing
these triangles has no effect on the 2D pixels or the collision silhouette.
