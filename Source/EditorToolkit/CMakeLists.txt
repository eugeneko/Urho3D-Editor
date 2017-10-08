# Automatically link Qt executables to qtmain target on Windows - don't need it.
cmake_policy (SET CMP0020 OLD)

# Find Qt
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
find_package (Qt5Core)
find_package (Qt5Widgets)
find_package (Qt5Xml)

# Add library
add_library(EditorToolkit
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/GenericUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/GenericUI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/KeyBinding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/KeyBinding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/UrhoUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/UrhoUI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/AbstractInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/AbstractInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/Qt/QtUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/Qt/QtUI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/Qt/QtUrhoWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/Qt/QtUrhoWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/Qt/QtUrhoHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericUI/Qt/QtUrhoHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/EditorViewportLayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/EditorViewportLayout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/CameraController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/CameraController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Editor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/EditorEvents.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/EditorInterfaces.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/EditorInterfaces.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/ObjectSelector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/ObjectSelector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Selection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Selection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/HierarchyWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/HierarchyWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/DebugGeometryRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/DebugGeometryRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Gizmo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Gizmo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Transformable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor/Transformable.h
)

# Link to Qt
target_link_libraries (EditorToolkit Qt5::Widgets Qt5::Xml)
