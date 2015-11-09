# VisIt-plugins

A set of plugins to read various snapshot formats. Currently supported:
- Gadget_custom: a plugin for Gadget2 type 2 snapshots that is slightly better than the one shipped with VisIt
- Shadowfax: plugin for the old Shadowfax snapshot format
- SWIZMO: plugin to read the Gadget2 type 3 snapshots, which are also used by GIZMO and SWIFT, and are the new default for Shadowfax

## Installation

First compile a new version of VisIt from source.
Download the build_visit script from https://wci.llnl.gov/simulation/computer-codes/visit/source (the latest version will do).
And run it:
```
./build_visit2_8_1 --console --hdf5 --szip
```
to download and compile the source code and required libraries.

Once that is done, put the folders in this repository under `src/databases/`.
Add the plugins to `src/databases/CMakeLists.txt`: either use the file in this repository, or add them manually (which
might be safer).

The Shadowfax and SWIZMO plugin go under
```
(CHECK_THIRDPARTY_DEPENDENT_PLUGINS(HDF5
```
in between `SAMRAI` and `SXRIS` (or similar).

The Gadget_custom plugin under
```
SET(REQUIRED_DATABASE_PLUGINS
```
in between `Gadget` and `Image` (or similar).

When you have added the desired plugins in this way, go back to the `src` folder and run `make`. This should compile
the plugins. They will be automatically loaded the next time you start VisIt.
