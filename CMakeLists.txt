#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#------------ lapkt PROJECT cmake script ------------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

# Check version requirement
cmake_minimum_required(VERSION 3.19)

project(lapkt VERSION 2.0)

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#---------- SECTION 1.0  PROJECT OPTIONS ------------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#


## USER CONFIGURABLE OPTIONS

#---- SECTION 1.2 Build Components  ----#
option(BUILD_CPP_TESTS "BUILD_CPP_TESTS" OFF)
option(BUILD_PYTHON_PKG "BUILD_PYTHON_PKG" OFF)

# include cmake modules

# LOG
message(STATUS "The CMAKE_INSTALL_PREFIX is " ${CMAKE_INSTALL_PREFIX})
message(STATUS "Building lapkt in " ${CMAKE_BUILD_TYPE} " mode")

#---- SECTION 1.1 CONFIGURE DEFAULT PARAMETER VALUES ----#

# Set policies

## NOTE: cmake adds new policies when it changes the interpretation of a 
## a function to match the evolving expectations of the developers. Policies are
## version dependent and an older version would not recognize the policy.

if(${CMAKE_VERSION} VERSION_LESS "3.24.0") 
  message(DEPRECATION "Please consider switching to the latest cmake version")
else()
  cmake_policy(SET CMP0135 NEW)
endif()

# Set dependency path variables

set(DEPS_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/_deps/install")
set(DEPS_BUILD_DIR "${PROJECT_BINARY_DIR}/_deps/build")

# Determine the build system
message(STATUS "THE LOCAL BUILD SYSTEM NAME IS : ${CMAKE_SYSTEM_NAME}")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX_BUILD ON)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOS_BUILD ON)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND MINGW)
  set(WINDOWS_BUILD ON)
else()
  message(FATAL_ERROR
"The build is only configured for Linux, Windows(MINGW), and Darwin(Macos)."
  )
endif()

set(CMAKE_CXX_STANDARD 17)

# Global flag to cause add_library() to create shared libraries
set(BUILD_SHARED_LIBS ON)

# static link with stdc++ and libgcc
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++")
  set(CMAKE_SHARED_LINKER_FLAGS  "-static-libstdc++") 
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
  set(CMAKE_SHARED_LINKER_FLAGS  "-static-libstdc++ -static-libgcc") 
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
  message(WARNING "The build for Intel compiler hasn't been tested. Proceed with caution")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  message(WARNING "The build for MSVC hasn't been tested. Proceed with caution")
endif()

if(MINGW)
  set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--allow-multiple-definition -static -lpthread") 
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-multiple-definition -static -lpthread")
endif(MINGW)

# https://stackoverflow.com/a/17032841/7248295
# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

#---- SECTION 1.2 CUSTOM METHODS ----#
function(cat FROM TO)
  file(READ ${FROM} CONTENTS)
  file(APPEND ${TO} "${CONTENTS}")
endfunction()

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#---- SECTION 2.0 - External packages/libraries -----#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

# @todo This should only be called if BUILD_PYTHON_PKG is ON
#---- Python  ----#

if(BUILD_PYTHON_PKG)

  if(MACOS_BUILD)
    set(Python_FIND_FRAMEWORK "FIRST")
    set(ENV{DYLD_FRAMEWORK_PATH} "/usr/local/Frameworks/")
  endif(MACOS_BUILD)
  
  if(NOT DEFINED PY_VERSION)
    find_package(Python COMPONENTS Interpreter Development)
  else()
    string(REPLACE "." ";" PY_VERSION_PART ${PY_VERSION})
    LIST(GET PY_VERSION_PART 0 PY_VMAJOR)
    LIST(GET PY_VERSION_PART 1 PY_VMINOR)
    MATH(EXPR PY_VMINOR_NEXT "${PY_VMINOR} + 1")
    # find_package(Python ${PY_VMAJOR}...<${PY_VMINOR} COMPONENTS Interpreter Development)
    find_package(Python ${PY_VMAJOR}.${PY_VMINOR}...<${PY_VMAJOR}.${PY_VMINOR_NEXT} 
      COMPONENTS Interpreter Development)
  endif()
  # if(${Python_VERSION} VERSION_LESS 3.6.0)
  #   message(SEND_ERROR 
  #     "UNTESTED PYTHON VERSION, expected >3.6.x but found - " ${Python_VERSION})
  # endif()

  message(STATUS "The PYTHON_VERSION is ${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}")

  #---- Pybind11  ----#

  if(NOT DEFINED pybind11_ROOT)
    message(STATUS "Building pybind11")
    file(GLOB RESULT ${PROJECT_SOURCE_DIR}/submodules/pybind11/*)
    list(LENGTH RESULT RES_LEN)
    if(RES_LEN EQUAL 0)
      message(FATAL_ERROR "LAPKT submodules have not been initialized, run - `git submodule update --jobs 20 --init --recurs
  ive` at the root of lapkt source")
    endif()
    execute_process(
      COMMAND ${CMAKE_COMMAND} -S ${PROJECT_SOURCE_DIR}/cmake/pybind11
      -B ${DEPS_BUILD_DIR}/pybind11
      -DDEPS_INSTALL_PREFIX=${DEPS_INSTALL_PREFIX}
      -DDEPS_BUILD_DIR=${DEPS_BUILD_DIR}
      -Dpybind11_SOURCE=${PROJECT_SOURCE_DIR}/submodules/pybind11
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
      -DPYTHON_INCLUDE_DIR=${Python_INCLUDE_DIRS}
      -DPYTHON_LIBRARY=${Python_LIBRARIES}
    )

    execute_process(
      COMMAND ${CMAKE_COMMAND} --build  ${DEPS_BUILD_DIR}/pybind11
    )

    set(pybind11_ROOT ${DEPS_INSTALL_PREFIX}/pybind11/)
  endif()

  message(STATUS "pybind11_ROOT: ${pybind11_ROOT}")

  # set(PYBIND11_NOPYTHON ON)
  find_package(pybind11 REQUIRED
    # As of cmake-v3.12 <PackageName>_ROOT is the first path that is checked
    PATHS ${pybind11_ROOT} 
    NO_DEFAULT_PATH
  )
endif() # BUILD_PYTHON_PKG


#---- Boost ----#
# Boost Configure and Build commands

if(NOT DEFINED Boost_ROOT)

  file(GLOB RESULT ${PROJECT_SOURCE_DIR}/submodules/boost/*)
  list(LENGTH RESULT RES_LEN)
  if(RES_LEN EQUAL 0)
    message(FATAL_ERROR "LAPKT submodules have not been initialized, run - `git submodule update --jobs 20 --init --recurs
ive` at the root of lapkt source")
  endif()

  set(CMAKE_BOOST_BUILD_TYPE "Release" CACHE STRING "Boost build type Debug/Release")

  execute_process(
    COMMAND ${CMAKE_COMMAND} -S ${PROJECT_SOURCE_DIR}/cmake/boost
    -B ${DEPS_BUILD_DIR}/boost
    -DDEPS_INSTALL_PREFIX=${DEPS_INSTALL_PREFIX}
    -DDEPS_BUILD_DIR=${DEPS_BUILD_DIR}
    -DBOOST_SOURCE=${PROJECT_SOURCE_DIR}/submodules/boost
    -DCMAKE_BOOST_BUILD_TYPE=${CMAKE_BOOST_BUILD_TYPE}
    -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
  )

  execute_process(
      COMMAND ${CMAKE_COMMAND} --build  ${DEPS_BUILD_DIR}/boost
  )

  # LOGS
  set(Boost_ROOT ${DEPS_INSTALL_PREFIX}/boost)
endif()

message(STATUS "Boost_ROOT: ${Boost_ROOT}")

# message(STATUS "CMAKE_MODULE_PATH is set to ${CMAKE_MODULE_PATH}")
set(Boost_VERBOSE ON)
set(Boost_DEBUG ON)

# Link with static Boost libraries instead of dynamic
option(CMAKE_STATIC_BOOST "Link with boost-c++ static libraries" ON)
if(CMAKE_STATIC_BOOST)
  set(Boost_USE_STATIC_LIBS ON)
  set(BOOST_PYTHON_STATIC_LIB ON)
endif(CMAKE_STATIC_BOOST)

message(STATUS "Boost_USE_STATIC_LIBS is ${Boost_USE_STATIC_LIBS}")
message(STATUS "BOOST_PYTHON_STATIC_LIB is ${BOOST_PYTHON_STATIC_LIB}")

list(APPEND Boost_Required_Components
  program_options
  random
  # headers
)
find_package(Boost 
  REQUIRED COMPONENTS ${Boost_Required_Components} 
  # As of cmake-v3.12 <PackageName>_ROOT is the first path that is checked
  # PATHS ${Boost_ROOT} 
  # NO_DEFAULT_PATH
)
message(STATUS "THE BOOST INCLUDE dirs search path is " ${Boost_INCLUDE_DIRS} )

#---- Catch2 ----#

if(BUILD_CPP_TESTS)
  if(NOT DEFINED Catch2_ROOT)
    message(STATUS "Building Catch2")
    file(GLOB RESULT ${PROJECT_SOURCE_DIR}/submodules/Catch2/*)
    list(LENGTH RESULT RES_LEN)
    if(RES_LEN EQUAL 0)
      message(FATAL_ERROR "LAPKT submodules have not been initialized, run - `git submodule update --jobs 20 --init --recurs
ive` at the root of lapkt source")
    endif()
    execute_process(
      COMMAND ${CMAKE_COMMAND} -S ${PROJECT_SOURCE_DIR}/cmake/Catch2
      -B ${DEPS_BUILD_DIR}/Catch2
      -DDEPS_INSTALL_PREFIX=${DEPS_INSTALL_PREFIX}
      -DDEPS_BUILD_DIR=${DEPS_BUILD_DIR}
      -DCATCH2_SOURCE=${PROJECT_SOURCE_DIR}/submodules/Catch2
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_GENERATOR=${CMAKE_GENERATOR}
    )

    execute_process(
      COMMAND ${CMAKE_COMMAND} --build  ${DEPS_BUILD_DIR}/Catch2
    )

    set(Catch2_ROOT ${DEPS_INSTALL_PREFIX}/Catch2)
  endif()

  find_package(Catch2 3 REQUIRED
  # As of cmake-v3.12 <PackageName>_ROOT is the first path that is checked
  # PATHS ${Catch2_ROOT} 
  # NO_DEFAULT_PATH
  )
  enable_testing()
endif()

#[[ Alternative FetchContent method to add Catch2 targets to the build
Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.0-preview4
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
]]

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#------ SECTION 3.0  lapkt LIBRARIES and DOCS -------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

#------------ SECTION 3.1  CONFIG OPTIONS -------------#
# PATHS relative to CMAKE_INSTALL_PREFIX and CMAKE_BINARY_DIR
# lapkt cxx library
# core and planner component
set(REL_CORE_LIB_DIR lib)
set(REL_CORE_INC_DIR include)
set(REL_CMAKE_DIR ${REL_CORE_LIB_DIR}/cmake/lapkt)
set(REL_PYEXT_LIB_DIR ${REL_CORE_LIB_DIR})
set(REL_PYEXT_INC_DIR ${REL_CORE_INC_DIR})
# pypi package
set(PYTHON_PKG_PATH ${CMAKE_INSTALL_PREFIX}/_pip_installable_pkg CACHE STRING "standalone package installation path")
set(REL_PYPI_ROOT _pip_installable_pkg)
set(REL_PYPI_LAPKT_ROOT ${REL_PYPI_ROOT}/site_package/lapkt)
# standalone package
set(STANDALONE_PKG_PATH ${CMAKE_INSTALL_PREFIX}/standalone_pkg CACHE STRING "standalone package installation path")

#[[
Note- When creating Python extensions, NEITHER the py extensions NOR the 
lapkt libs need to be linked to Python shared libraries. Including the
headers is sufficient. The reference to Python symbols will be populated by
when the library is loaded by the python interpreter. This would free the 
python extensions from VERSION specific Python shared libs.
]]

#------------- SECTION 3.2  BUILD--------------#
#[[
The 'lapkt' library is a pure cxx library containing the core structures,
and algorithms. 
All non-c++(py) extensions should link with the 'core' shared library. 
This library should have minimal dependencies. 
]]

#---- DECLARE library targets and set properties ----#

#---- core - pure cxx component ----#
if(LINUX_BUILD)
  # set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};$ORIGIN")
elseif(WINDOWS_BUILD)
  # set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH};$ORIGIN")
elseif(MACOS_BUILD)
  message(STATUS "Setting up RPATH on macos build")

  message(STATUS "CMAKE_INSTALL_NAME_DIR @loader_path")
  set(CMAKE_INSTALL_NAME_DIR @loader_path)
endif()

add_library(core SHARED)
add_library(lapkt::core ALIAS core)

target_link_libraries(core PRIVATE
  ${Boost_LIBRARIES}
)

# set_target_properties(core PROPERTIES
#     #### @todo - Doesn't work in Windows(MINGW)
#   LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${REL_CORE_LIB_DIR}
# )

#---- Python extensions ----#

if(BUILD_PYTHON_PKG)

  #---- CXX Wrappers over LAPKT lib ----#

  if(WINDOWS_BUILD)
    pybind11_add_module(wrapper SHARED)
  elseif(LINUX_BUILD)
    add_library(wrapper SHARED)
    target_link_libraries(wrapper PUBLIC
      pybind11::pybind11
    )
  endif()

  set_target_properties(wrapper PROPERTIES
    PREFIX ""
    INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN" #@todo CHECK THIS FOR MACOS
  )

  target_link_libraries(wrapper PRIVATE
    core
    Boost::boost
  )

  #---- Translate PDDL ----#
  if(WINDOWS_BUILD)
    pybind11_add_module(pddl SHARED)
  elseif(LINUX_BUILD)
    add_library(pddl SHARED)
  endif()
  set_target_properties(pddl PROPERTIES
    PREFIX ""
    INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN"
  )
  target_link_libraries(pddl PRIVATE
    core
    wrapper
    Boost::boost
  )

  #---- Planner ----#
  if(WINDOWS_BUILD)
    pybind11_add_module(planner SHARED)
  elseif(LINUX_BUILD)
    add_library(planner SHARED)
  endif()

  set_target_properties(planner PROPERTIES
    PREFIX ""
    INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN"
  )

  target_link_libraries(planner PRIVATE
    core
    pybind11::pybind11
    Boost::boost
  )
endif() # BUILD_PYTHON_PKG

#---- Add sub directories with source ----#

# Add src
add_subdirectory(src)

# Unit tests
if(BUILD_CPP_TESTS)
  add_subdirectory(ctest)
endif()


#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#----------- SECTION 4.0  INSTALL CXXLIBS -----------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

# install boost headers

install(
  DIRECTORY
  ${Boost_INCLUDE_DIRS}/
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${REL_CORE_INC_DIR}/
  COMPONENT
    lapkt
)

# core
install(TARGETS core 
  EXPORT lapkt-core-Targets
  DESTINATION ${REL_CORE_LIB_DIR}/lapkt
  COMPONENT lapkt
  INCLUDES DESTINATION
    ${REL_CORE_INC_DIR}
)

# Allows the developers to use find_package(lapkt COMPONENT core) 
#   to link lapkt library from their cmake project
install(
  EXPORT      lapkt-core-Targets
  FILE        lapkt-core-Targets.cmake
  DESTINATION ${REL_CMAKE_DIR}
  COMPONENT lapkt
  NAMESPACE lapkt::
)

if(BUILD_PYTHON_PKG)

  # wrapper
  install(TARGETS wrapper
    EXPORT lapkt-wrapper-Targets
    LIBRARY DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    COMPONENT lapkt    
    ARCHIVE DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    COMPONENT lapkt    
    RUNTIME  DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    COMPONENT lapkt    
    INCLUDES DESTINATION    
      ${REL_PYEXT_INC_DIR}
  )
  # Allows the developers to use find_package(lapkt COMPONENT planner) 
  #   to link lapkt library from their cmake project
  # install(
  #   EXPORT      lapkt-wrapper-Targets
  #   FILE        lapkt-wrapper-Targets.cmake
  #   DESTINATION ${REL_CMAKE_DIR}
  #   COMPONENT lapkt
  #   NAMESPACE lapkt::
  # )
  # planner
  install(TARGETS planner
    EXPORT lapkt-planner-Targets
    DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    LIBRARY DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    COMPONENT lapkt    
    ARCHIVE DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    COMPONENT lapkt    
    RUNTIME  DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    COMPONENT lapkt    
    INCLUDES DESTINATION    
      ${REL_PYEXT_INC_DIR}
  )
  # # Allows the developers to use find_package(lapkt COMPONENT planner) 
  # #   to link lapkt library from their cmake project
  # install(
  #   EXPORT      lapkt-planner-Targets
  #   FILE        lapkt-planner-Targets.cmake
  #   DESTINATION ${REL_CMAKE_DIR}
  #   COMPONENT lapkt
  #   NAMESPACE lapkt::
  # )
  # # pddl
  install(TARGETS pddl
    EXPORT lapkt-pddl-Targets
    LIBRARY DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    COMPONENT lapkt    
    ARCHIVE DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    COMPONENT lapkt    
    RUNTIME  DESTINATION ${REL_PYEXT_LIB_DIR}/lapkt
    COMPONENT lapkt    
    INCLUDES DESTINATION    
      ${REL_PYEXT_INC_DIR}
  )
  # Allows the developers to use find_package(lapkt COMPONENT pddl) 
  #   to link lapkt library from their cmake project
  # install(
  #   EXPORT      lapkt-pddl-Targets
  #   FILE        lapkt-pddl-Targets.cmake
  #   DESTINATION ${REL_CMAKE_DIR}
  #   COMPONENT lapkt
  #   NAMESPACE lapkt::
  # )
endif() #BUILD_PYTHON_PKG

#---- SECTION 4.2 Install Configuration File

## Allow developers using the lapkt library to fetch information, including library version
configure_file(${PROJECT_SOURCE_DIR}/cmake/lapkt_cmake_config.h.in 
  lapkt_cmake_config.h
)

install(FILES "${PROJECT_BINARY_DIR}/lapkt_cmake_config.h"
  DESTINATION ${REL_CORE_INC_DIR}/lapkt
  COMPONENT lapkt 
)
include(CMakePackageConfigHelpers)
# generate the config file that includes the exports
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
  ${PROJECT_BINARY_DIR}/lapktConfig.cmake
  INSTALL_DESTINATION ${REL_CMAKE_DIR}
  # NO_SET_AND_CHECK_MACRO
  # NO_CHECK_REQUIRED_COMPONENTS_MACRO
  # PATH_VARS INCLUDE_INSTALL_DIR 
)
# generate the version file for the config file
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/lapktConfigVersion.cmake"
  VERSION "${lapkt_VERSION_MAJOR}.${lapkt_VERSION_MINOR}"
  COMPATIBILITY AnyNewerVersion
) 
install(FILES
  ${PROJECT_BINARY_DIR}/lapktConfig.cmake
  ${PROJECT_BINARY_DIR}/lapktConfigVersion.cmake
  DESTINATION ${REL_CMAKE_DIR}
  COMPONENT lapkt
)

#---- SECTION 4.2 CPACK CONFIGURATIOn

# include(InstallRequiredSystemLibraries)
# set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")
# set(CPACK_PACKAGE_VERSION_MAJOR "${lapkt_VERSION_MAJOR}")
# set(CPACK_PACKAGE_VERSION_MINOR "${lapkt_VERSION_MINOR}")
# include(CPack)

#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#
#--------- SECTION 5.0  COMPILE PYTHON PKG.  --------#
#XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX#

# - Compile python package

if(BUILD_PYTHON_PKG)

  install(DIRECTORY
      ${PROJECT_BINARY_DIR}/${REL_PYPI_ROOT}/
      USE_SOURCE_PERMISSIONS
      DESTINATION ${PYTHON_PKG_PATH}
      COMPONENT PYPACKAGE EXCLUDE_FROM_ALL 
  )

  install(PROGRAMS
  ${PROJECT_SOURCE_DIR}/src/python_pkg/lapkt.py
      DESTINATION ${STANDALONE_PKG_PATH}
      COMPONENT PYPACKAGE EXCLUDE_FROM_ALL 
  )

  install(DIRECTORY
    ${CMAKE_INSTALL_PREFIX}/${REL_CORE_INC_DIR}
    ${CMAKE_INSTALL_PREFIX}/${REL_CORE_LIB_DIR}
    USE_SOURCE_PERMISSIONS
    DESTINATION ${PYTHON_PKG_PATH}/site_package/lapkt/core/
    COMPONENT PYPACKAGE 
  )

  install(DIRECTORY
    ${PYTHON_PKG_PATH}/site_package/lapkt
    USE_SOURCE_PERMISSIONS
    DESTINATION ${STANDALONE_PKG_PATH}
    COMPONENT PYPACKAGE
  )
endif() #BUILD_PYTHON_PKG



if(BUILD_PYTHON_PKG)

  add_custom_target(foo ALL)
  add_dependencies(foo wrapper planner pddl core)

  add_custom_command(
    TARGET foo POST_BUILD
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --component lapkt
  )

  add_custom_command(
    TARGET foo POST_BUILD
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --component PYPACKAGE
  )
  # The script will run during the installation(We want this at the bottom of all install target)
  ## Install lapkt using python pip module
  # install(SCRIPT ${PROJECT_SOURCE_DIR}/cmake/PostInstallScript.cmake)

else()
  add_custom_command(
    TARGET core POST_BUILD
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --component lapkt
  )

endif() # BUILD_PYTHON_PKG