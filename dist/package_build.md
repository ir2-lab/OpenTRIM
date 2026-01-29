# OpenTRIM - Package build

The following table shows packages to be built and their dependencies (rpm style)

| Package        | Type          | Requires      | BuildRequires  |
| -------------- | ------------- | ------------- | -------------- |
| libdedx        | Shared Lib    | -             | -              |
| libdedx-devel  | Lib interface | libdedx       | -              |
| qmatplotwidget | Static Lib    | qwt-qt5       | qwt-qt5-devel  |
| qtdatabrowser  | Static Lib    | qwt-qt5       | qmatplotwidget |
| opentrim       | Executable    | libdedx       | libdedx-devel  |
|                |               |               | eigen-devel    |
|                |               | hdf5          | hdf5-devel     |
| opentrim-gui   | Executable    | opentrim      |                |
|                |               | qwt-qt5       | qtdatabrowser  |
| opentrim-devel | Lib interface | opentrim      |                |
|                |               | libdedx-devel |                |
|                |               | eigen-devel   |                |
  