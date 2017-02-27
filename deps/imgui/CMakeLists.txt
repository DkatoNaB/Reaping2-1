set(LIBDBG_URL
    "https://bitbucket.org/edd/imgui/get/0.2.1.tar.gz"
    CACHE STRING "Location of imgui source package")

ExternalProject_Add(ext_imgui
        PREFIX "${EXTERNALS_WORK_DIR}"
        SOURCE_DIR "${EXTERNALS_WORK_DIR}/src/imgui"
        BINARY_DIR "${EXTERNALS_WORK_DIR}/build/imgui"
        INSTALL_DIR "${EXTERNALS_INSTALL_DIR}"
        GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
        GIT_TAG "adb85d800d70d3ddc4548f0e8f184eede4568034"
        PATCH_COMMAND
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_CMakeLists.txt
                    ${EXTERNALS_WORK_DIR}/src/imgui/CMakeLists.txt
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/FindIMGUI.cmake
            ${EXTERNALS_WORK_DIR}/src/imgui/FindIMGUI.cmake
        CMAKE_ARGS
            ${PLATFORM_CMAKE_ARGS}
            ${TOOLCHAIN_CMAKE_ARGS}
            -G ${CMAKE_GENERATOR}
            -DCMAKE_INSTALL_PREFIX=${EXTERNALS_INSTALL_DIR}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

