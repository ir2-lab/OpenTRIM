# OpenTRIM - Package build

OpenTRIM and its subprojects:

- [libdedx](https://github.com/ir2-lab/libdedx) 
- [QMatPlotWidget](https://github.com/gapost/qmatplotwidget) 
- [QtDataBrowser](https://github.com/ir2-lab/QtDataBrowser) 

are organized in a number of packages as shown below. 

Mutual dependencies or external dependencies are also given.

---
## Packages

1. **libdedx / libdedx-devel** — Shared Lib and dev files
   - Requires: —
   - BuildRequires: —

2. **qmatplotwidget / qmatplotwidget-devel** — Shared Lib and dev files
   - Requires: qwt-qt5
   - BuildRequires: qwt-qt5-devel

3. **qtdatabrowser / qtdatabrowser-devel** — Shared Lib
   - Requires: qmatplotwidget
   - BuildRequires: qmatplotwidget-devel

4. **libopentrim** — Shared Lib and dev files
   - Requires: libdedx, hdf5
   - BuildRequires: libdedx-devel, eigen3-devel, hdf5-devel

5. **libopentrim-devel** — Lib interface
   - Requires: libopentrim, libdedx-devel, eigen3-devel
   - BuildRequires: libopentrim, libdedx-devel, eigen3-devel, hdf5-devel

6. **opentrim** — Executable
   - Requires: libopentrim
   - BuildRequires: libopentrim-devel

7. **opentrim-gui** — Executable
   - Requires: libopentrim, qtdatabrowser
   - BuildRequires: qtdatabrowser-devel, libopentrim-devel
