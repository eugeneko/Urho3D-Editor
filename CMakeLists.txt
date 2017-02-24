project (Urho3DEditor)
cmake_minimum_required(VERSION 3.2.3)

# Setup Urho3D

cmake_policy (SET CMP0003 NEW)
# INTERFACE_LINK_LIBRARIES defines the link interface
cmake_policy (SET CMP0022 NEW)
# Disallow use of the LOCATION target property - so we set to OLD as we still need it
cmake_policy (SET CMP0026 OLD)
# MACOSX_RPATH is enabled by default
cmake_policy (SET CMP0042 NEW)
# Automatically link Qt executables to qtmain target on Windows - don't need it.
cmake_policy (SET CMP0020 OLD)

set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)
include (Urho3D-CMake-common)

# Setup Editor

add_subdirectory (${CMAKE_SOURCE_DIR}/Source/Urho3DEditor)
include_directories (${CMAKE_SOURCE_DIR}/Source)

# Setup Launcher

find_package (Qt5Core)
set (CMAKE_AUTOMOC ON)
set (SOURCE_FILES ${CMAKE_SOURCE_DIR}/Source/EditorLauncher/Main.cpp)
qt5_add_resources(SOURCE_FILES ${CMAKE_SOURCE_DIR}/Resources/QDarkStyle/QDarkStyle.qrc ${CMAKE_SOURCE_DIR}/Resources/Editor/Editor.qrc)
set (TARGET_NAME EditorLauncher)
setup_main_executable ()
target_link_libraries (EditorLauncher Urho3DEditor)
