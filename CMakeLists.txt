cmake_minimum_required(VERSION 3.16)

project(vtkit VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(vtkit SHARED
    src/vtkit.c
)

option(VTK_BUILD_STATIC "Build a static vtkit library in addition to the shared library" ON)

if(VTK_BUILD_STATIC)
    add_library(vtkit_static STATIC
        src/vtkit.c
    )
endif()

target_include_directories(vtkit
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(VTK_BUILD_STATIC)
    target_include_directories(vtkit_static
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endif()

target_compile_features(vtkit PUBLIC c_std_99)

if(VTK_BUILD_STATIC)
    target_compile_features(vtkit_static PUBLIC c_std_99)
endif()

target_compile_definitions(vtkit
    PRIVATE VTK_BUILD
)

set_target_properties(vtkit PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME vtkit
)

if(VTK_BUILD_STATIC)
    set_target_properties(vtkit_static PROPERTIES
        OUTPUT_NAME vtkit_static
    )
endif()

if(WIN32)
    target_link_libraries(vtkit PRIVATE ws2_32)
    if(VTK_BUILD_STATIC)
        target_link_libraries(vtkit_static PRIVATE ws2_32)
    endif()
endif()

set(VTK_INSTALL_TARGETS vtkit)
if(VTK_BUILD_STATIC)
    list(APPEND VTK_INSTALL_TARGETS vtkit_static)
endif()

install(TARGETS ${VTK_INSTALL_TARGETS}
    EXPORT vtkitTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES include/vtkit/vtkit.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vtkit
)

install(EXPORT vtkitTargets
    FILE vtkitTargets.cmake
    NAMESPACE vtkit::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vtkit
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/vtkitConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    cmake/vtkitConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/vtkitConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vtkit
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vtkitConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/vtkitConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vtkit
)

option(VTK_BUILD_EXAMPLE "Build the example consumer executables" ON)

if(VTK_BUILD_EXAMPLE)
    add_executable(vtk_demo examples/demo.c)
    target_link_libraries(vtk_demo PRIVATE vtkit)

    add_executable(vtk_pong examples/pong.c)
    target_link_libraries(vtk_pong PRIVATE vtkit)

    add_executable(vtk_lines examples/lines.c)
    target_link_libraries(vtk_lines PRIVATE vtkit)

    add_executable(vtk_boxes examples/boxes.c)
    target_link_libraries(vtk_boxes PRIVATE vtkit)
endif()
