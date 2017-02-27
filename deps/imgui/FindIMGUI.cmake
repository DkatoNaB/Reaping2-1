cmake_minimum_required(VERSION 2.8)

find_path(IMGUI_INCLUDE_DIR imgui.h)

set(IMGUI_NAMES ${IMGUI_NAMES} imgui imguid libimgui libimguid)
find_library(IMGUI_LIBRARY NAMES ${IMGUI_NAMES} PATH)

if(WIN32)
    find_library(IMM_LIBRARY NAMES imm32 libimm32 PATH)
endif()

if(IMGUI_LIBRARY)
    LIST( APPEND IMGUI_LIBRARIES ${IMGUI_LIBRARY} ${IMM_LIBRARY})
endif()

if(IMGUI_INCLUDE_DIR AND IMGUI_LIBRARY)
    set(IMGUI_FOUND TRUE)
endif()

if(IMGUI_FOUND)
    if(NOT Imgui_FIND_QUIETLY)
        message(STATUS "Found imgui: ${IMGUI_LIBRARIES}")
    endif()
else(IMGUI_FOUND)
    if(Imgui_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find imgui")
    endif()
endif(IMGUI_FOUND)
