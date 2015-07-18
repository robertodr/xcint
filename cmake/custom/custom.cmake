include(ExternalProject)


# FIXME
option(ENABLE_FORTRAN_INTERFACE "Build Fortran interface" OFF)
if(ENABLE_FORTRAN_INTERFACE)
    enable_language(C CXX Fortran)
else()
    enable_language(C CXX)
endif()

# XCFun code
set(ExternalProjectCMakeArgs
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/external
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DENABLE_FORTRAN_INTERFACE=OFF
    -DENABLE_STATIC_LINKING=ON # we need the -fPIC
    )
ExternalProject_Add(xcfun
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/xcfun
    BINARY_DIR ${PROJECT_BINARY_DIR}/external/xcfun-build
    STAMP_DIR ${PROJECT_BINARY_DIR}/external/xcfun-stamp
    TMP_DIR ${PROJECT_BINARY_DIR}/external/xcfun-tmp
    INSTALL_DIR ${PROJECT_BINARY_DIR}/external
    CMAKE_ARGS ${ExternalProjectCMakeArgs}
    )


# numgrid code
set(numgrid_args
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/external
    )
ExternalProject_Add(numgrid
    SOURCE_DIR ${PROJECT_SOURCE_DIR}/external/numgrid
    BINARY_DIR ${PROJECT_BINARY_DIR}/external/numgrid-build
    STAMP_DIR ${PROJECT_BINARY_DIR}/external/numgrid-stamp
    TMP_DIR ${PROJECT_BINARY_DIR}/external/numgrid-tmp
    INSTALL_DIR ${PROJECT_BINARY_DIR}/external
    CMAKE_ARGS ${numgrid_args}
    )
include_directories(${PROJECT_SOURCE_DIR}/external/numgrid/api)


include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/density
    ${PROJECT_BINARY_DIR}/external/include
    ${PROJECT_SOURCE_DIR}/external/googletest
    ${PROJECT_SOURCE_DIR}/external/googletest/include
    ${PROJECT_SOURCE_DIR}/api
    ${PROJECT_BINARY_DIR}/generated
    )

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/generated)


# generate AO evaluation code
add_custom_command(
    OUTPUT
        ${PROJECT_BINARY_DIR}/generated/autogenerated.h
        ${PROJECT_BINARY_DIR}/generated/autogenerated_0.cpp
        ${PROJECT_BINARY_DIR}/generated/autogenerated_1.cpp
        ${PROJECT_BINARY_DIR}/generated/autogenerated_2.cpp
        ${PROJECT_BINARY_DIR}/generated/autogenerated_3.cpp
        ${PROJECT_BINARY_DIR}/generated/autogenerated_4.cpp
        ${PROJECT_BINARY_DIR}/generated/autogenerated_5.cpp
        ${PROJECT_BINARY_DIR}/generated/parameters.h
        ${PROJECT_BINARY_DIR}/generated/offsets.h
    COMMAND
        ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/src/density/generate.py ${PROJECT_BINARY_DIR}/generated
    WORKING_DIRECTORY
        ${PROJECT_SOURCE_DIR}/src/density
    DEPENDS
        src/density/generate.py
        src/density/cs_trans.py
    )


# generate ave_contributions.h
add_custom_command(
    OUTPUT
        ${PROJECT_BINARY_DIR}/generated/ave_contributions.h
    COMMAND
        ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/src/generate_ave_contributions.py > ${PROJECT_BINARY_DIR}/generated/ave_contributions.h
    DEPENDS
        src/generate_ave_contributions.py
    )
add_custom_target(
    generate_ave
    ALL
    DEPENDS
        ${PROJECT_BINARY_DIR}/generated/ave_contributions.h
    )


add_library(
    xcint
    src/rolex.cpp
    src/Functional.cpp
    src/integrator.cpp
    src/MemAllocator.cpp
    src/density/ao_vector.cpp
    src/density/Basis.cpp
    src/density/AOBatch.cpp
    ${PROJECT_BINARY_DIR}/generated/autogenerated_0.cpp
    ${PROJECT_BINARY_DIR}/generated/autogenerated_1.cpp
    ${PROJECT_BINARY_DIR}/generated/autogenerated_2.cpp
    ${PROJECT_BINARY_DIR}/generated/autogenerated_3.cpp
    ${PROJECT_BINARY_DIR}/generated/autogenerated_4.cpp
    ${PROJECT_BINARY_DIR}/generated/autogenerated_5.cpp
    )

set_target_properties(xcint PROPERTIES COMPILE_FLAGS "-fPIC")

add_library(
    xcint_shared
    SHARED
    src/empty.cpp
    )

target_link_libraries(
    xcint_shared
    ${MATH_LIBS}
    "-Wl,--whole-archive"
    xcint
    ${PROJECT_BINARY_DIR}/external/lib/libxcfun.a
    "-Wl,--no-whole-archive"
    )

add_dependencies(xcint xcfun)
add_dependencies(xcint numgrid)

if(ENABLE_FORTRAN_INTERFACE)
    add_library(
        xcint_fortran_api
        api/xcint_fortran_api.F90
        )
endif()

install(TARGETS xcint ARCHIVE DESTINATION lib)
install(TARGETS xcint_shared LIBRARY DESTINATION lib)
