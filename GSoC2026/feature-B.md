# Feature B - Real-Time 3D Visualization for OpenTRIM
## GSoC 2026 · Technical Design

## Table of Contents

1. Purpose
2. Design Philosophy
3. Physical Track Model
4. Architecture and Data Flow
5. Obtaining Track Data
6. The 3D Scene
7. Rendering
8. Limits, Playback and Controls
9. GUI Integration
10. Build System
11. File Organization
12. PR Plan
13. Library Modifications
14. Testing
15. Future Work

---

## 1. Purpose

Draw ion tracks in 3D, live, while a simulation runs in the GUI.

An ion track is a zigzag of straight free-flight segments. At each vertex the ion hits a target
atom, transfers recoil energy, and changes direction. If the recoil energy is above the
displacement threshold, the struck atom starts its own track. The primary ion's energy is shared
among all recoils until no more displacements occur. This is a displacement cascade.

The viewer serves three purposes:
- help the user understand the problem under study
- inspect and debug simulation geometry and material configuration
- produce images and movies for talks and papers

The GUI today shows 2D Qwt slices (`ResultsView`) and a static geometry slice (`SimBoxView`).
Nothing shows the cascade in 3D. SRIM's 2D track view was influential in the 90s. OVITO is a
modern post-processing tool for such images, but it works on saved data and favors quality over
speed. Our view must update live during a run, so efficiency comes first.

```
Before:  run simulation → "Simulation Data" tab → 2D Qwt depth profile (post-run)
After:   run simulation → "3D Vis" tab → live 3D cascade tracks, colored and playing back in time
```

Feature B is GUI-only. It lives in `source/gui/`. It does not touch the Python bindings
(`bindings/python/`).

---

## 2. Design Philosophy

| Principle | What it means in practice |
|-----------|--------------------------|
| **Use the core event handler** | Track data is read through `mcdriver::install_event_handler()`, the API added for this feature. No new hooks in the physics code. |
| **Minimal work on the sim thread** | The handler only copies a few fields from the `const ion&` into a plain buffer and signals the GUI. It never renders or allocates on the hot path. |
| **Decouple view from sim rate** | Simulation speed ranges from ~100 ions/s (heavy ions) to ~1M ions/s (light ions). The view draws one cascade every one to several seconds. Only a fraction of cascades are shown. |
| **Bounded memory** | Energy, recoil-generation and memory limits cap what is stored. A single VBO holds the drawable tracks. |
| **Qt5 OpenGL** | `QOpenGLWidget`, `QOpenGLShaderProgram`, `QMatrix4x4`. No third-party GL dependency. |
| **Two classes** | `Track3DViewport` owns the OpenGL and the data link. `TrackViewWidget` owns the controls around it. |

---

## 3. Physical Track Model

One track is the sequence of the start point and every collision:

```
(x0,t0), (x1,t1), ... , (xN,tN)
```

Per vertex we also keep the energy, the atom species, and the recoil generation (0 for the
source ion, +1 for each new recoil generation). A track is drawn as a line strip through its
vertices. A cascade is the source ion plus every recoil it produces, directly or indirectly.

A single source-ion history produces this event sequence, which the viewer reassembles into
tracks:

```
NewSourceIon              source ion track starts
  Scattering  × N
  <track end>             IonStop | IonExit | Replacement
NewRecoil                 first queued recoil starts
  Scattering  × N
  <track end>
NewRecoil                 next recoil ...
  ...
<queue empty>            source-ion history (cascade) complete
NewSourceIon              next source ion, next cascade
```

---

## 4. Architecture and Data Flow

```
┌──────────────────────────────────────────────────────────────────────────┐
│  Worker thread 0   mccore::run()                                         │
│      event handler (installed on thread 0) receives each masked Event    │
│      copies (pos, energy, time, recoil-gen, atom-id) into a sync buffer  │
│      buffer full or cascade end → queued signal to the GUI               │
└───────────────────────────────┬──────────────────────────────────────────┘
                                │  Qt::QueuedConnection (sim thread → GUI thread)
┌───────────────────────────────┴──────────────────────────────────────────┐
│  GUI (main) thread   Track3DViewport                                     │
│      take the ready vertex arrays, append to the OpenGL VBO              │
│      record each track's start offset and vertex count                   │
│      paintGL(): glMultiDrawArrays(GL_LINE_STRIP), time-evolution playback│
└──────────────────────────────────────────────────────────────────────────┘
```

The handler runs on the simulation thread. The one careful part of the whole feature is this single crossing from the simulation thread to the GUI thread, done with a queued signal so no lock is held on the hot path.

Only one thread needs the handler, since only a fraction of cascades are drawn. Thread 0 is
chosen because it exists even in single-threaded runs.

| Layer | Class | File |
|---|---|---|
| Track data source | `mcdriver` event handler | core (already provided) |
| Handler + GL + VBO | `Track3DViewport` | `source/gui/track3dviewport.cpp` (new) |
| Controls + layout | `TrackViewWidget` | `source/gui/trackviewwidget.cpp` (new) |

---

## 5. Obtaining Track Data

George added the extraction API to the core (on `GSoC2026`, commit `d5b10eb`). We use it as is.

### The event handler API

```cpp
// mccore.h:143 - the handler signature
typedef void (*event_handler)(Event ev, const ion& i, void* p);

// mcdriver.h:435 - install on one worker thread
bool install_event_handler(mccore::event_handler h, uint32_t mask,
                           void* p = nullptr, int thread_no = 0);
void clear_event_handlers();               // mcdriver.h:441
```

At `exec()` the driver installs each registered handler on its clone:
`sim_clones_[eh.thread_no]->set_event_handler(...)` (`mcdriver.cpp:207-209`). Inside the core,
`handle_event` is gated by `globalEventMask_` (`mccore.h:491`) and dispatches to the handler in
`handle_event_impl` (`mccore.cpp:597`). Events are off unless some consumer sets the mask, so the
handler adds nothing to a CLI or Python run.

### The events we need are now fired

- `NewSourceIon` / `NewRecoil` at track start - `mccore.cpp:372`
  (`handle_event(recoil_id() ? NewRecoil : NewSourceIon, *i)`).
- `Scattering` at each collision - `mccore.cpp:542, 563`.
- Track end - `IonStop` (`mccore.cpp:388`), `IonExit` (`mccore.cpp:418, 455`),
  `Replacement` (`mccore.cpp:526`).

The visualization mask is `NewSourceIon | NewRecoil | Scattering | IonStop | IonExit |
Replacement`.

### What the handler does

Everything for a vertex is on the `ion` object:

| Field | Source | Unit |
|-------|--------|------|
| `x, y, z` | `ion::pos()` (ion.h:142) | nm |
| `energy` | `ion::erg()` (ion.h:151) | eV |
| `t` | `ion::t()` (ion.h:157) | ns |
| `rid` (recoil generation) | `ion::recoil_id()` (ion.h:187) | - |
| `aid` (atom id) | `ion::myAtom()->id()` (target.h:125) | - |

The handler is a static function in `Track3DViewport`. `McDriverObj` registers it once, right
after it creates the driver (`mcdriverobj.cpp:331`, `driver_ = mcdriver::create(opt)`), so it is
installed before the simulation starts. While the 3D tab is inactive the handler stays in a
pass-through mode with negligible cost.

When the user activates the view, the handler waits for the next `NewSourceIon` and starts
accumulating from there, so it always begins at a cascade boundary. It then follows the sequence
in Section 3, collecting vertices until the following `NewSourceIon`, which marks the current
cascade complete and ready to hand off. It copies fields only, then signals the GUI. Double
buffering keeps the simulation and the view from stalling each other.

---

## 6. The 3D Scene

The scene contains:
1. The simulation bounding box, in a neutral black or grey.
2. The material regions, as a weakly transparent tint, using the per-material colors already in
   the config (`target.h:219`, e.g. `"#55aaff"`).
3. A user-selectable number of ion cascades.

The box and regions are always visible. Cascade drawing starts and stops with play/pause, in two
modes:
- **Fixed batch** - draw up to N cascades, then stop; the user can restart.
- **Ring buffer** - cycle through N cascades, dropping the oldest as a new one arrives; runs
  until the simulation ends.

Tracks appear progressively as the cascade unfolds, so the user sees it develop in time. Ion
energies, and so timescales, span orders of magnitude, so a log or a fictitious playback
timescale may be needed to keep on-screen activity roughly uniform. This gets checked while
coding.

---

## 7. Rendering

The layout below is indicative. OpenGL allows several ways to do this.

### Vertex layout

```cpp
struct TrackVertex {
    float   x, y, z;   // nm
    float   energy;    // eV
    float   t;         // ns
    int16_t rid;       // recoil generation
    int16_t aid;       // atom id
};  // 24 bytes
```

One cascade can reach ~100k vertices (~2.3 MB). A single pre-allocated VBO, e.g. 100 MB, holds a
good number of them.

### Draw call

Tracks are appended with `glBufferSubData`; each track's start offset and vertex count are
recorded, and all tracks are drawn in one call:

```cpp
// append a track (V is this track's vertex array)
GLint   first = pos / sizeof(TrackVertex);              // vertex index, not byte offset
GLsizei count = GLsizei(V.size());                      // vertex count for this track
glBufferSubData(GL_ARRAY_BUFFER, pos, V.size() * sizeof(TrackVertex), V.data());
track_first.push_back(first);
track_count.push_back(count);
pos += V.size() * sizeof(TrackVertex);

// draw all tracks in one call
glMultiDrawArrays(GL_LINE_STRIP, track_first.data(), track_count.data(),
                  GLsizei(track_first.size()));
```

### Shaders

Time evolution is done in the fragment shader: a vertex whose time is past the current playback
time is discarded. Color coding is a colormap over the chosen attribute.

```glsl
// fragment shader
uniform float uTime;      // playback time
in float vEnergy;
in float vT;
out vec4 fragColor;

void main() {
    if (vT > uTime) discard;        // time evolution
    fragColor = colormap(vEnergy);  // or recoil-gen / species
}
```

`QOpenGLShaderProgram` manages the shaders, `QMatrix4x4` the view and projection, and `paintGL`
schedules the next frame while playing.

### Color modes

Track color is user-selectable by ion energy, recoil generation, or atomic species. Colormaps
should contrast with the material background colors. Because energy spans a wide range, the
energy scale supports a log mapping. A colorbar shows the active mapping. It can sit as a child
widget of `TrackViewWidget` next to the viewport, or as an overlay in the OpenGL scene. Qwt's
`QwtScaleWidget`, already used in the GUI (`simboxview.cpp:118`), can render it.

---

## 8. Limits, Playback and Controls

A cascade can hold a very large number of tracks, and low-energy recoils are numerous and add
clutter. Three independent limits are active at once:
- ion energy threshold (e.g. hide recoils starting below 1 keV)
- recoil generation cutoff (e.g. up to the 5th generation)
- memory cap on accumulated vertex data (e.g. 100 MB)

Changing any limit clears the buffer.

The user controls:
- camera: rotate, pan, zoom; preset views and a "home" button; save/load camera state to JSON
- cascade mode (fixed batch or ring buffer) and cascade count (1 to about 10)
- play / pause
- clear (delete all tracks)
- playback timescale / speed
- color mode and colormap, with a colorbar
- the three limits above
- screenshot to file
- movie recording (optional)

---

## 9. GUI Integration

A new tab is added below "Config". Icon from ionicons or lucide.dev. Short title "3D Vis", long
title "3D Visualization". Tabs are added with `push(title, widget)` (`mainui.cpp:180`; existing
tabs at `mainui.cpp:55, 89-100`).

The feature is two classes:

**`Track3DViewport`** - a `QOpenGLWidget` subclass. It owns all OpenGL code and the data link to
the simulation. It defines the static event handler and exposes slots and properties to drive the
view:

```cpp
class Track3DViewport : public QOpenGLWidget {
    Q_OBJECT
    Q_PROPERTY(int nCascades READ nCascades WRITE setNCascades)
    // ...
public slots:
    void play(bool on);        // start / stop playback
    // ...
};
```

`McDriverObj` registers the viewport's handler unconditionally when it creates the driver, via
`mcdriver::install_event_handler`. The handler notifies `Track3DViewport` on the GUI thread with
a queued connection. This is the one cross-thread point that needs care.

**`TrackViewWidget`** - a `QWidget` shown in the main area for the "3D Vis" tab. It holds the
`Track3DViewport` plus the controls. Primary controls (play/pause, screenshot) sit below the
viewport; the rest go in a right-side panel, grouped by a tab widget into View, Playback, Color,
Limits. It connects the controls to the viewport via signals and slots.

Camera state is serialized with `nlohmann::json` (consistent with the rest of the codebase, no
Qt JSON).

---

## 10. Build System

The GUI is Qt5. It links `Qt5::Widgets` and `Qt5::Svg` (`source/gui/CMakeLists.txt:57-58`), plus
Qwt and nlohmann_json, but no OpenGL module. All present views use Qwt for 2D plotting
(`simboxview.h:12`). There is no OpenGL code in the repository today. Feature B introduces the
first `QOpenGLWidget`.

`QOpenGLWidget` is in the Widgets module; the `QOpenGL*` helpers are in the Gui module, which
Widgets pulls in. B-2 adds the OpenGL link and the shader resources:

```cmake
# source/gui/CMakeLists.txt - addition (B-2)
find_package(Qt5 REQUIRED COMPONENTS Widgets Svg OpenGL)
target_link_libraries(${GUI_TARGET} PRIVATE Qt5::OpenGL)
# shaders bundled via a Qt resource (.qrc)
```

> The exact module, `Qt5::OpenGL` or the `Qt5::Gui` that Widgets already pulls in, depends on the
> local Qt5 version. We pin it in B-2. Flagged because the GUI has never linked GL.

No change to the Python build (`bindings/python/`) or the `libopentrim` link flags.

---

## 11. File Organization

`[B]` marks new or modified files. Feature B is entirely under `source/gui/`.

```
source/gui/
├── track3dviewport.h/.cpp   ← QOpenGLWidget: GL, VBO, event handler, camera   [B] new
├── trackviewwidget.h/.cpp   ← QWidget: viewport + controls panel               [B] new
├── shaders/
│   ├── track.vert           ← position/energy/time/rid/aid → gl_Position       [B] new
│   └── track.frag           ← time-evolution discard + colormap                [B] new
├── mcdriverobj.h/.cpp       ← install_event_handler after mcdriver::create     [B]
├── mainui.cpp               ← add "3D Vis" tab                                  [B]
├── CMakeLists.txt           ← Qt5::OpenGL, new sources, shader .qrc            [B]
└── md/track_viewer_guide.md ← user guide                                       [B] new
```

No changes to `source/lib/`, `source/include/`, `source/cli/`, or `bindings/`.

---

## 12. PR Plan

6 PRs, one per week, second half of the GSoC period. Feature A ran Weeks 1-5 before the midterm;
Feature B runs Weeks 7-12 before the final. Every PR lives in `source/gui/`.

```
Week 7   B-1  Data Pipeline                     Jul 11–17
         Register the event handler on thread 0 via mcdriver::install_event_handler.
         Static handler in Track3DViewport, sync/double buffer, queued signal to GUI.
         No GL yet: cascade vertex arrays arrive on the GUI thread.

Week 8   B-2  Viewport and Scene                Jul 18–24
         Track3DViewport (QOpenGLWidget). Qt5::OpenGL in the build. Camera with
         rotate/pan/zoom, presets and home. Bounding box + material-region tint. New "3D Vis" tab.

Week 9   B-3  Track Rendering                   Jul 25–31
         VBO + glMultiDrawArrays(GL_LINE_STRIP). Vertex/fragment shaders. Time-evolution
         playback. play/pause/clear. Fixed-batch and ring-buffer modes.

Week 10  B-4  Color and Limits                  Aug 1–7
         Color by energy / recoil generation / species, log energy scale, colorbar.
         Energy, generation and memory limits (clear buffer on change).

Week 11  B-5  Controls and Reproducibility      Aug 8–14
         TrackViewWidget controls panel (View / Playback / Color / Limits).
         Camera-state JSON save/load. Screenshot to file.

Week 12  B-6  Documentation                     Aug 15–17
         User guide with annotated screenshots.

         ────── Feature B complete before final ──────
         Final evaluation: Aug 17–24
```

### PR Table

| PR | Title | Delivers | Key Files |
|----|-------|----------|-----------|
| **B-1** | `feat: 3D track data pipeline via event handler` | Handler on thread 0, sync/double buffer, queued signal, cascade vertex arrays on GUI thread | `mcdriverobj.h/.cpp` ✎ · `track3dviewport.h/.cpp` ✚ |
| **B-2** | `feat: Track3DViewport - QOpenGLWidget + camera + scene` | 3D tab, Qt5::OpenGL, camera (presets/home), bounding box + material tint | `track3dviewport.cpp` ✎ · `mainui.cpp` ✎ · `CMakeLists.txt` ✎ |
| **B-3** | `feat: track rendering + time-evolution playback` | glMultiDrawArrays line strips, shaders, playback, batch/ring modes | `track3dviewport.cpp` ✎ · `shaders/track.vert` ✚ · `shaders/track.frag` ✚ |
| **B-4** | `feat: color modes, colorbar and limits` | Energy/gen/species color, log energy, colorbar, three limits | `track3dviewport.cpp` ✎ · `shaders/track.frag` ✎ |
| **B-5** | `feat: control panel, camera JSON, screenshot` | TrackViewWidget layout, camera save/load, screenshot | `trackviewwidget.h/.cpp` ✚ · `track3dviewport.cpp` ✎ |
| **B-6** | `docs: 3D viewer user guide` | Guide: controls, modes, color, limits | `md/track_viewer_guide.md` ✚ |

> ✚ = new file · ✎ = modified existing file

---

## 13. Library Modifications

None. The core work was done by George on `GSoC2026` (commit `d5b10eb`): the missing events
(`NewRecoil`, `Scattering`) now fire, and `mcdriver::install_event_handler` provides the
extraction path. Feature B uses this API and stays entirely in `source/gui/`.

The only GUI-side core-adjacent call is `McDriverObj` registering the viewport handler after
`mcdriver::create` (`mcdriverobj.cpp:331`). No change to `mccore`, `mcdriver`, the HDF5 format,
the tally math, or the Python bindings.

---

## 14. Testing

| Test | Checks | Method |
|------|--------|--------|
| Handler capture | vertex arrays non-empty after N ions with the mask set | unit test with a small run |
| Transparency | CLI and bindings output unchanged when no handler is installed | run a b-case with and without the GUI |
| Cascade assembly | vertex sequence matches the event order in Section 3 | replay a recorded event stream |
| Ring buffer | oldest cascade dropped when count exceeds N; batch mode stops at N | overflow test |
| Memory limit | accumulation stops and buffer clears at the cap | forced small cap |
| Render sanity | VBO vertex count > 0 after 100 ions; frame renders | automated check |
| Screenshot | saved PNG is non-empty and not blank | pixel-variance check |

CI note: OpenGL rendering needs a GL context. On a headless CI runner, use Mesa software GL
(`LIBGL_ALWAYS_SOFTWARE=1`); otherwise keep the render tests local.

---

## 15. Future Work

| Feature | Why deferred |
|---------|-------------|
| SEM convergence indicator | It belongs to simulation results, not track visuals. A candidate for the Summary tab later. |
| Post-run replay from HDF5 | OpenTRIM may save track events (NewSourceIon/NewRecoil/Scattering) to HDF5; users could build their own render workflow. Datasets can reach several GB. |
| Headless / scripted rendering | Would require linking the CLI executable to OpenGL and Qt, beyond its purpose. Likely a separate tool. |
| Movie recording | Optional; screenshot ships first. |
| Multi-tally heatmap | Cut earlier to keep the viewer focused. |

---
