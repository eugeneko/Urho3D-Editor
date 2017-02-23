project (Urho3DEditor)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_AUTOMOC ON)
find_package (Qt5Widgets)
find_package (Qt5Xml)

file (GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.cpp
)

foreach (SOURCE_FILE ${SOURCE_FILES})
    string (REPLACE "${PROJECT_SOURCE_DIR}" "" PATH ${SOURCE_FILE})
    get_filename_component (PATH ${PATH} PATH)
    if (PATH)
        string (REPLACE / \\ PATH ${PATH})
        source_group ("${PATH}" FILES ${SOURCE_FILE})
    else ()
        source_group ("Source" FILES ${SOURCE_FILE})
    endif ()
endforeach ()

include_directories (${URHO3D_HOME}/include)
include_directories (${URHO3D_HOME}/include/Urho3D/ThirdParty)
add_library (Urho3DEditor ${SOURCE_FILES})
target_link_libraries (Urho3DEditor Qt5::Widgets Qt5::Xml) 
