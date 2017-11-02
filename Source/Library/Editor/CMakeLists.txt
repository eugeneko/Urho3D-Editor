# Add library
add_library(Editor
    ${CMAKE_CURRENT_SOURCE_DIR}/EditorViewportLayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EditorViewportLayout.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CameraController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CameraController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Editor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/EditorEvents.h
    ${CMAKE_CURRENT_SOURCE_DIR}/EditorInterfaces.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EditorInterfaces.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ObjectSelector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ObjectSelector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Selection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Selection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/HierarchyWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HierarchyWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugGeometryRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugGeometryRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Gizmo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Gizmo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Inspector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Inspector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ResourceBrowser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResourceBrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Transformable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Transformable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UndoStack.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UndoStack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/StandardEditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StandardEditor.h
)

# Link to AbstractUI
target_link_libraries (Editor AbstractUI)
