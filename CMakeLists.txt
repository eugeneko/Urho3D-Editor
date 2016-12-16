cmake_minimum_required(VERSION 3.2.3)

project (Urho3DEditor)

#
# Setup Qt
#

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
find_package (Qt5Widgets)

#
# Setup Urho3D
#

if (WIN32)
    cmake_minimum_required (VERSION 3.2.3)      # Going forward all platforms will use this as minimum version
else ()
    cmake_minimum_required (VERSION 2.8.6)
endif ()
if (COMMAND cmake_policy)
    cmake_policy (SET CMP0003 NEW)
    if (CMAKE_VERSION VERSION_GREATER 2.8.12 OR CMAKE_VERSION VERSION_EQUAL 2.8.12)
        # INTERFACE_LINK_LIBRARIES defines the link interface
        cmake_policy (SET CMP0022 NEW)
    endif ()
    if (CMAKE_VERSION VERSION_GREATER 3.0.0 OR CMAKE_VERSION VERSION_EQUAL 3.0.0)
        # Disallow use of the LOCATION target property - so we set to OLD as we still need it
        cmake_policy (SET CMP0026 OLD)
        # MACOSX_RPATH is enabled by default
        cmake_policy (SET CMP0042 NEW)
    endif ()
endif ()

set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake/Modules)
include (Urho3D-CMake-common)

#
# Setup Editor Library
#

set (SOURCE_DIR ${CMAKE_SOURCE_DIR}/Source)
file (GLOB_RECURSE EDITOR_SOURCE
    ${SOURCE_DIR}/Editor/*.h
    ${SOURCE_DIR}/Editor/*.cpp
)

foreach (SOURCE_FILE ${EDITOR_SOURCE})
    string (REPLACE "${SOURCE_DIR}" "" PATH ${SOURCE_FILE})
    get_filename_component (PATH ${PATH} PATH)
    if (PATH)
        string (REPLACE / \\ PATH ${PATH})
        source_group ("${PATH}" FILES ${SOURCE_FILE})
    else ()
        source_group ("Source" FILES ${SOURCE_FILE})
    endif ()
endforeach ()

add_library (Urho3DEditor ${EDITOR_SOURCE})
target_link_libraries (Urho3DEditor Qt5::Widgets)

#
# Setup Editor Launcher
#

set (TARGET_NAME Urho3DEditorLauncher)
set (SOURCE_FILES ${CMAKE_SOURCE_DIR}/Source/EditorLauncher/Main.cpp)
setup_main_executable ()
target_link_libraries (Urho3DEditorLauncher Urho3DEditor)

