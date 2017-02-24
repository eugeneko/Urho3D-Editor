# Urho3D Editor

Native Qt-based editor library for [Urho3D](https://github.com/urho3d/Urho3D) and related projects.

Work in progress.

## License

Do What The Fuck You Want To

_TODO: Update license to something like MIT_

## Installation

### Required libraries

1) Qt 5.7.1 or later;

2) Latest revision of Urho3D master branch or fork with relevant commits.

   _TODO: [This branch](https://github.com/eugeneko/Urho3D/tree/editor-update) is actually required now_

   _TODO: Bind Editor to release version_

### Building

1) Set `URHO3D_HOME`;

2) Add path to Qt (e.g. `C:/Qt/Qt5.7.1/5.7/msvc2015`) to `CMAKE_PREFIX_PATH`;

3) Generate and build root CMake project.

4) Ensure that Qt5Core, Qt5Widgets and Qt5Xml debug and release libraries are  successfully located.

   E.g. add path to Qt binaries (e.g. `C:/Qt/Qt5.7.1/5.7/msvc2015/bin`) to `PATH` or copy binaries manually.
   
   _TODO: Use `windeployqt`_

### Using Editor as external library

1) Create typical application that is linked against `Urho3D`, check Urho3D Wiki for example.

2) Set CMake policy `CMP0020` to `OLD`:

   `cmake_policy (SET CMP0020 OLD)`
   
3) Add CMake project of Editor library located in `Source/Urho3DEditor`:

   `add_subdirectory (path/to/Urho3D-Editor/Source/Urho3DEditor)`
   
   Note that `add_subdirectory` could work not only with subdirectories.

4) `find_package (Qt5Core)`

   Just do it.

5) Link to Editor resources:

   `qt5_add_resources(${SOURCE_FILES} path/to/Urho3D-Editor/Resources/QDarkStyle/QDarkStyle.qrc path/to/Urho3D-Editor/Resources/Editor/Editor.qrc)`

6) Optionally link to some other resources.

Example of resulting `CMakeLists.txt` is the root CMake project.
