project (Urho3DEditor)

# Setup CMake

cmake_minimum_required(VERSION 3.2.3)
cmake_policy (SET CMP0003 NEW)
# INTERFACE_LINK_LIBRARIES defines the link interface
cmake_policy (SET CMP0022 NEW)
# Disallow use of the LOCATION target property - so we set to OLD as we still need it
cmake_policy (SET CMP0026 OLD)
# MACOSX_RPATH is enabled by default
cmake_policy (SET CMP0042 NEW)

# Setup Qt

set (CMAKE_AUTOMOC ON)
find_package (Qt5Core)

# Setup Urho3D

set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake/Modules)
include (Urho3D-CMake-common)

# Setup Editor

add_subdirectory (${PROJECT_SOURCE_DIR}/Source/Urho3DEditor)

# Setup Launcher

qt5_add_resources(EDITOR_RESOURCES ${PROJECT_SOURCE_DIR}/Resources/QDarkStyle/QDarkStyle.qrc ${PROJECT_SOURCE_DIR}/Resources/Editor/Editor.qrc)
set (SOURCE_FILES ${PROJECT_SOURCE_DIR}/Source/EditorLauncher/Main.cpp ${EDITOR_RESOURCES})
set (TARGET_NAME EditorLauncher)
setup_main_executable ()
target_link_libraries (EditorLauncher Urho3DEditor)
