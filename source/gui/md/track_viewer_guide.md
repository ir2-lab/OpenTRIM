This is a short guide to the **3D Visualization** panel, which draws the ion and
recoil tracks of a running simulation in real time.

Open the viewer with the **"3D Vis"** button on the left. Click the **"?"** button
on the viewer toolbar to open this guide. The panel has five parts:

![The 3D Visualization panel](./images/track_viewer_panel.png)

  1. the **3D scene** with the target box and the captured tracks,
  2. the **toolbar** for capture, screenshots and preset views,
  3. the **color legend**, which matches the current color mode,
  4. the **control tabs** ("Capture", "Color", "Camera") for changing various settins,
  5. the **status panel**, reporting how much data is currently held.

### 1. Capturing tracks

The 3D visualization component captures ion tracks from the running simulation
and displays them in the 3D scene.

Capturing is on by default when a simulation is created and can be switched off or restarted at any time
by the user. Capturing is active only while the **"3D Vis"** tab is selected and the 3D scene visible. 

Track events are stored in a bounded memory buffer; the size of the buffer and the number of displayed tracks
is user selectable.

The capturing process can be controlled by the toolbar buttons:

  - **"Capture on/off"** manually disables or re-enables capture. Its label
    tracks the state ("Capture on", "Capture off", "Paused") and it blinks while
    a capture is finishing or pausing.
  - **"Clear"** drops all tracks from memory.

More detailed control is available in the **"Capture"** tab, where the following options can be set:

  - **Buffer Mode**
    - **Ring** (default) captures continuously in a buffer of the size set below.
      When the buffer is full, the oldest cascade is dropped.
    - **Batch** captures until the buffer is full, then stops.
  - **Speed [ps/s]** is the playback rate: simulation time advances at this many
    nanoseconds per real second. The slider is logarithmic.
  - **Buffer Size**
    - **Cascades** is the maximum number of cascades to keep.
    - **Mem [MB]** is an upper bound on the memory used.
  - **Ion track thresholds**
    - **E min [eV]** ion tracks below this energy are not displayed.
    - **Max Recoil gen.** drops recoils above this generation. "all" keeps every
      generation.

Changing any of the buffer size or threshold options clears the current buffer.

### 2. Navigating the scene

Using the mouse:
  - **Left drag** changes the camera view angle.
  - **Right drag** or **middle drag** pans the view.
  - **Wheel** zooms in/out.

The toolbar has preset view buttons: a home/isometric view, and the six axis
views (top, bottom, front, back, left, right).

### 3. Coloring tracks

The **"Color"** tab sets how tracks are colored. The legend (3) always matches
the current mode.

  - **Mode**
    - **Recoil Generation** colors each track by its generation (source ion,
      1, 2, 3, 4+).
    - **Energy** colors each point by the ion energy at that point.
    - **Atomic Species** colors each track by its atomic species of the moving ion.
  - **Color Map** selects different color mapping options. Energy offers continuous maps
    (Ramp, Rainbow, Turbo); the generation and species modes offer discrete
    palettes (Default, Tab10).
  - **Energy scale** applies to the Energy mode.
    - **Log E** switches between a linear and a logarithmic scale.
    - **Auto Scale** fits the scale to the data. Turn it off to set **min [eV]**
      and **max [eV]** by hand.

![A close cascade view, colored by species](./images/track_viewer_cascade.png)

### 4. Playback

The captured ion cascades are shown as they evolve in simulation time. 
The playback rate is set by the **Speed [ps/s]** value on
the "Capture" tab. The status panel (5) reports the current play time and the
time span held in the buffer.

### 5. Saving

  - The **camera icon** on the toolbar saves a high resolution screenshot of the 3D scene to a
    PNG file.
  - In the **"Camera"** tab, the current camera state can be saved to a JSON file and restored 
    later, so a fixed viewpoint can be reused across runs.
