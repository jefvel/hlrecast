cmake_minimum_required(VERSION 3.12)
project(recast)

option(RECAST_NAVIGATION_PATH "Path to recastnavigation repository" ../../recastnavigation)

function(set_as_hdll target)
    set_target_properties(${target}.hdll
        PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${target}
        SUFFIX .hdll
    )
endfunction()

include_directories(../../hashlink/src)
include_directories(${RECAST_NAVIGATION_PATH}/Recast/Include)

#link_directories(src/bullet/bin)

add_definitions(-DRECAST_EXPORTS)
add_definitions(-fpermissive)

link_directories(/usr/local/lib)
include(GNUInstallDirs)

set(HDLL_DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
)

set(INCLUDES_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(RECAST_SRC_DIR ${RECAST_NAVIGATION_PATH}/Recast/Source)

add_library(recast.hdll SHARED
    ${RECAST_SRC_DIR}/Recast.cpp
    ${RECAST_SRC_DIR}/RecastAlloc.cpp
    ${RECAST_SRC_DIR}/RecastArea.cpp
    ${RECAST_SRC_DIR}/RecastAssert.cpp
    ${RECAST_SRC_DIR}/RecastContour.cpp
    ${RECAST_SRC_DIR}/RecastFilter.cpp
    ${RECAST_SRC_DIR}/RecastLayers.cpp
    ${RECAST_SRC_DIR}/RecastMesh.cpp
    ${RECAST_SRC_DIR}/RecastMeshDetail.cpp
    ${RECAST_SRC_DIR}/RecastRasterization.cpp
    ${RECAST_SRC_DIR}/RecastRegion.cpp
    src/recast.cpp
)


target_include_directories(recast.hdll
    PRIVATE
    ${RECAST_SRC_DIR}
)


set_as_hdll(recast)
target_link_libraries(recast.hdll
    hl
)
set_target_properties(recast.hdll PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ../../hashlink)


if (!WIN32)
install(
    TARGETS
        recast.hdll
    DESTINATION ${HDLL_DESTINATION}
)
endif()
