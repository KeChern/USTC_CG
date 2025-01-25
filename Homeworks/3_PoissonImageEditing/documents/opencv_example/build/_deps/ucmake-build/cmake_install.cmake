# Install script for directory: D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Ubpa")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-build/src/test/00_Basic/cmake_install.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/UCMake_0_6_0/cmake" TYPE FILE FILES
    "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-build/UCMakeConfig.cmake"
    "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-build/UCMakeConfigVersion.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/UCMake_0_6_0" TYPE DIRECTORY FILES "D:/2020CG/USTC_CG-master/Homeworks/3_PoissonImageEditing/documents/opencv_example/build/_deps/ucmake-src/cmake")
endif()

