set (SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/StandardEditor.cpp ${CMAKE_CURRENT_SOURCE_DIR}/StandardEditor.h)
set (TARGET_NAME 00_Editor)
setup_main_executable ()
target_link_libraries (00_Editor Editor AbstractUI)
