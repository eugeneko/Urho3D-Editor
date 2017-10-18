# Automatically link Qt executables to qtmain target on Windows - don't need it.
cmake_policy (SET CMP0020 OLD)

# Find Qt
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
find_package (Qt5Core)
find_package (Qt5Widgets)

# Add library
add_library(AbstractUI
    ${CMAKE_CURRENT_SOURCE_DIR}/AbstractUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AbstractUI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/KeyBinding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/KeyBinding.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AbstractInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AbstractInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Urho/UrhoUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Urho/UrhoUI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Qt/QtUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Qt/QtUI.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Qt/QtUrhoWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Qt/QtUrhoWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Qt/QtUrhoHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Qt/QtUrhoHelpers.h
)

# Link to Qt
target_link_libraries (AbstractUI Qt5::Widgets)
