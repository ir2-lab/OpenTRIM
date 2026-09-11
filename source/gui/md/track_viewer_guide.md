This is a short guide to the **3D Visualization** panel, which draws ion and
recoil tracks from a running simulation in real time.

Open the viewer with the **"3D Vis"** button on the left. The panel has five parts:

  1. the **3D scene** with the target box and the captured tracks,
  2. the **view toolbar** for selectig view angle, taking screenshots and toggle the buffer info panel,
  3. the **capture/playback toolbar**,
  4. the **colorbar** showing color info,
  5. the **settings tab** for changing various settings, open from the capture toolbar.

![The 3D Visualization panel](./images/track_viewer_panel.png)

### Capturing tracks

The 3D visualization component captures ion tracks from the running simulation
and displays them in the 3D scene.

Capturing is on by default when a simulation is created and can be switched off or restarted at any time by the user. Capturing is active only while the **"3D Vis"** tab is selected and the 3D scene visible. 

Track events are stored in a bounded memory buffer; the size of the buffer and the number of displayed tracks is user selectable.

In playback mode, the captured ion cascades are shown as they evolve in simulation time. 
The playback rate is set by the toolbar dropdown. The status panel reports the current play time and the time span held in the buffer.

The capturing/playback process can be controlled by the toolbar buttons:

  - ![Rec](./images/rec_button.png) Start/stop track capture
  - ![Play](./images/play-circle-outline.png) Start/stop playback of stored tracks
  - ![Back](./images/play-back-circle-outline.png) ![Fwd](./images/play-forward-circle-outline.png) Move playback time back/forward
  - ![Clear](./images/close-circle-outline.png) Clear captured tracks from memory
  - ![Settings](./images/options-outline.png) Open the settings tab

### Navigating the scene

Using the mouse:
  - **Left drag** changes the camera view angle.
  - **Right drag** or **middle drag** pans the view.
  - **Wheel** zooms in/out.

The view toolbar has preset view buttons: a home/isometric view, and the six axis
views (top, bottom, front, back, left, right).

### Settings

#### 1. 3D memory buffer

The following options can be set:

  - **Buffer Mode**
    - **Ring** (default) captures continuously in a buffer of the size set below.
      When the buffer is full, older cascades are dropped to make room for the new ones.
    - **Batch** captures until the buffer is full, then stops.
  - **Buffer Size**
    - **Cascades** is the maximum number of cascades to keep.
    - **Mem [MB]** is an upper bound on the memory used.
  - **Ion track thresholds**
    - **E min [eV]** ion tracks below this energy are not displayed/stored.
    - **Max Recoil gen.** drops recoils above this generation. "all" keeps every
      generation.

Changing any of the buffer size or threshold options clears the current buffer.

#### 2. Color

The **"Color"** tab sets how tracks are colored. The colorbar (4) always matches
the current mode.

  - **Mode**
    - **Recoil Generation** colors each track by its generation (source ion - 0,
      PKA - 1, 2, 3, 4+).
    - **Energy** colors each point on the track by the ion energy at that point.
    - **Atomic Species** colors each track by the atomic species of the moving ion.
  - **Color Map** selects different color mapping options. Energy offers continuous maps
    (Rainbow, Turbo); the generation and species modes offer discrete
    palettes (Tab10, Set1).
  - **Energy scale** applies to the Energy mode.
    - **Log E** switches between a linear and a logarithmic scale.
    - **Auto Scale** fits the scale to the data. Turn it off to set **min [eV]**
      and **max [eV]** by hand.

#### 3. Camera State

The current camera state can be saved to a JSON file and restored later, so that a fixed viewpoint can be reused across runs.
