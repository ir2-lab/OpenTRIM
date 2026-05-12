# OpenTRIM - Package build

OpenTRIM and its subprojects:

- [libdedx](https://github.com/ir2-lab/libdedx) 
- [QMatPlotWidget](https://github.com/gapost/qmatplotwidget) 
- [QtDataBrowser](https://github.com/ir2-lab/QtDataBrowser) 

are organized in a number of packages as shown below. 

Mutual dependencies or external dependencies are also given.

---
## Packages

1. **libdedx** — Shared Lib
   - Requires: —
   - BuildRequires: —

2. **libdedx-devel** — Lib interface
   - Requires: libdedx
   - BuildRequires: —

3. **qmatplotwidget-devel** — Static Lib
   - Requires: qwt-qt5-devel
   - BuildRequires: qwt-qt5-devel

4. **qtdatabrowser-devel** — Static Lib
   - Requires: qmatplotwidget-devel
   - BuildRequires: qmatplotwidget-devel

5. **libopentrim** — Shared Lib
   - Requires: libdedx, hdf5
   - BuildRequires: libdedx-devel, eigen3-devel, hdf5-devel

6. **libopentrim-devel** — Lib interface
   - Requires: libopentrim, libdedx-devel, eigen3-devel
   - BuildRequires: libopentrim, libdedx-devel, eigen3-devel, hdf5-devel

7. **opentrim** — Executable
   - Requires: libopentrim
   - BuildRequires: libopentrim-devel

8. **opentrim-gui** — Executable
   - Requires: libopentrim, qwt-qt5
   - BuildRequires: qtdatabrowser-devel, libopentrim-devel
