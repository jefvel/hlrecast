cmake_minimum_required(VERSION 3.12)
project(recast)

set(RECAST_NAVIGATION_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../recastnavigation" CACHE PATH "Path to recastnavigation repository")
set(HASHLINK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../hashlink" CACHE PATH "Path to where hashlink is")
set(HASHLINK_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../hashlink/$(Configuration)" CACHE PATH "Path to where hashlink libraries are")

function(set_as_hdll target)
    set_target_properties(${target}.hdll
        PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${target}
        SUFFIX .hdll
    )
endfunction()

include_directories(${HASHLINK_PATH}/src)

include_directories(${RECAST_NAVIGATION_PATH}/Recast/Include)
include_directories(${RECAST_NAVIGATION_PATH}/Detour/Include)
include_directories(${RECAST_NAVIGATION_PATH}/DetourTileCache/Include)
include_directories(${RECAST_NAVIGATION_PATH}/DebugUtils/Include)

link_directories(${HASHLINK_LIB_PATH})

add_definitions(-DRECAST_EXPORTS)

if (!WIN32)
add_definitions(-fpermissive)
link_directories(/usr/local/lib)
endif()

include(GNUInstallDirs)

set(HDLL_DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
)


set(RECAST_SRC_DIR ${RECAST_NAVIGATION_PATH}/Recast/Source)
set(RECAST_INC_DIR ${RECAST_NAVIGATION_PATH}/Recast/Include)
set(DETOUR_SRC_DIR ${RECAST_NAVIGATION_PATH}/Detour/Source)
set(DETOUR_TILE_CACHE_SRC_DIR ${RECAST_NAVIGATION_PATH}/DetourTileCache/Source)

set(DEBUG_UTILS_SRC_DIR ${RECAST_NAVIGATION_PATH}/DebugUtils/Source)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/native)
include_directories(${SRC_DIR})

add_library(recast.hdll SHARED
    # Recast Files
    ${RECAST_SRC_DIR}/Recast.cpp
    ${RECAST_INC_DIR}/Recast.h
    ${RECAST_SRC_DIR}/RecastAlloc.cpp
    ${RECAST_INC_DIR}/RecastAlloc.h
    ${RECAST_SRC_DIR}/RecastArea.cpp
    ${RECAST_SRC_DIR}/RecastAssert.cpp
    ${RECAST_INC_DIR}/RecastAssert.h
    ${RECAST_SRC_DIR}/RecastContour.cpp
    ${RECAST_SRC_DIR}/RecastFilter.cpp
    ${RECAST_SRC_DIR}/RecastLayers.cpp
    ${RECAST_SRC_DIR}/RecastMesh.cpp
    ${RECAST_SRC_DIR}/RecastMeshDetail.cpp
    ${RECAST_SRC_DIR}/RecastRasterization.cpp
    ${RECAST_SRC_DIR}/RecastRegion.cpp

    # Detour Files
    ${DETOUR_SRC_DIR}/DetourAlloc.cpp
    ${DETOUR_SRC_DIR}/DetourAssert.cpp
    ${DETOUR_SRC_DIR}/DetourCommon.cpp
    ${DETOUR_SRC_DIR}/DetourNavMesh.cpp
    ${DETOUR_SRC_DIR}/DetourNavMeshBuilder.cpp
    ${DETOUR_SRC_DIR}/DetourNavMeshQuery.cpp
    ${DETOUR_SRC_DIR}/DetourNode.cpp

    # Detour Tile Cache Files
    ${DETOUR_TILE_CACHE_SRC_DIR}/DetourTileCache.cpp
    ${DETOUR_TILE_CACHE_SRC_DIR}/DetourTileCacheBuilder.cpp

    # Debug Utils Files
    ${DEBUG_UTILS_SRC_DIR}/DebugDraw.cpp
    ${DEBUG_UTILS_SRC_DIR}/DetourDebugDraw.cpp
    ${DEBUG_UTILS_SRC_DIR}/RecastDebugDraw.cpp
    ${DEBUG_UTILS_SRC_DIR}/RecastDump.cpp

    ${SRC_DIR}/recast.cpp
    
)

target_include_directories(recast.hdll
    PRIVATE
    ${RECAST_SRC_DIR}
)

set_as_hdll(recast)
target_link_libraries(recast.hdll
    libhl
)
set_target_properties(recast.hdll PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${HASHLINK_PATH}
)

if(WIN32)
    set(HDLL_DESTINATION "C:/GameDev/hl/")
endif()

install(
    TARGETS
        recast.hdll
    DESTINATION ${HDLL_DESTINATION}
)
