link_libraries(glfw)
link_libraries(mechanica)
link_libraries(${LLVM_LIBRARIES})
link_libraries(${OPENGL_LIBRARIES})

include_directories(${glfw_INCLUDE_DIRS} "${GLFW_SOURCE_DIR}/deps")


if (MATH_LIBRARY)
    link_libraries("${MATH_LIBRARY}")
endif()

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (GLFW_USE_OSMESA)
    add_definitions(-DUSE_NATIVE_OSMESA)
endif()

if (WIN32)
    set(ICON hmm.rc)
elseif (APPLE)
    set(ICON hmm.icns)
    set_source_files_properties(hmm.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
endif()

set(GLAD "${GLFW_SOURCE_DIR}/deps/glad/glad.h"
         "${GLFW_SOURCE_DIR}/deps/glad.c")
set(GETOPT "${GLFW_SOURCE_DIR}/deps/getopt.h"
           "${GLFW_SOURCE_DIR}/deps/getopt.c")
set(TINYCTHREAD "${GLFW_SOURCE_DIR}/deps/tinycthread.h"
  "${GLFW_SOURCE_DIR}/deps/tinycthread.c")

add_executable(test_particles1 WIN32 MACOSX_BUNDLE test_particles1.c ${ICON} ${GLAD})
add_executable(test_particles2 WIN32 MACOSX_BUNDLE test_particles2.c ${ICON} ${GLAD})
add_executable(test_particles3 WIN32 MACOSX_BUNDLE test_particles3.c ${ICON} ${GLAD})

set(WINDOWS_BINARIES test_particles1 test_particles2 test_particles3)

#set_target_properties(${WINDOWS_BINARIES} ${CONSOLE_BINARIES} PROPERTIES
#                      FOLDER "GLFW3/Examples")

if (MSVC)
    # Tell MSVC to use main instead of WinMain for Windows subsystem executables
    set_target_properties(${WINDOWS_BINARIES} PROPERTIES
                          LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

if (APPLE)
  set_target_properties(test_particles1 PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Test 1")
  set_target_properties(test_particles2 PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Test 2")
  set_target_properties(test_particles3 PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "Test 3")


  set_target_properties(${WINDOWS_BINARIES} PROPERTIES
    RESOURCE glfw.icns
    MACOSX_BUNDLE_ICON_FILE hmm.icns
    MACOSX_BUNDLE_INFO_PLIST "${GLFW_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in")
endif()

