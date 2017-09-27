set (SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Urho3DPlayer.h ${CMAKE_CURRENT_SOURCE_DIR}/Urho3DPlayer.cpp)
set (TARGET_NAME Urho3DPlayer)
setup_main_executable ()
target_link_libraries (Urho3DPlayer EditorToolkit)
