cmake_minimum_required(VERSION 3.20)

project(Engine)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SRC_FILES 
    "core/*"
    "Drivers/*"
    "Editor/*"
    "Interface/*"
    "Platform/*"
    "Renderer/*"
    "Scene/*"
    "Main/main.cpp"
    "thirdParty/*"
)

set(ALL_INCLUDE_DIRS "")

foreach(FILE_PATH ${SRC_FILES})
    if(${FILE_PATH} MATCHES ".*\\.(h|hpp)$")
        get_filename_component(DIR_PATH ${FILE_PATH} DIRECTORY)
        list(APPEND ALL_INCLUDE_DIRS ${DIR_PATH})
    endif()
endforeach()

if(ALL_INCLUDE_DIRS)
    list(REMOVE_DUPLICATES ALL_INCLUDE_DIRS)
endif()

add_executable(Engine ${SRC_FILES})

target_compile_features(Engine PRIVATE cxx_std_20)

target_include_directories(Engine PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ALL_INCLUDE_DIRS}
)

target_link_libraries(Engine PRIVATE 
    d3d11.lib 
    dxgi.lib 
    d3dcompiler.lib
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})
