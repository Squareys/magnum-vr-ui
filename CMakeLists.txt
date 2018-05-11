#
#   Copyright © 2018 Jonathan Hale <squareys@googlemail.com>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

cmake_minimum_required(VERSION 2.8.12)

# CMake policies. 0025 needs to be before project(), so putting all there.
# Use AppleClang instead of Clang on Apple
if(POLICY CMP0025)
    cmake_policy(SET CMP0025 NEW)
endif()
# Don't treat imported targets with :: as files
if(POLICY CMP0028)
    cmake_policy(SET CMP0028 NEW)
endif()
# Enable MACOSX_RPATH by default
if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
endif()
# Quoted variables should not be dereferenced
if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

# Configuration fails on < 3.4 if only C++ is enabled (CheckFunctionExists
# macro called from FindX11)
if(NOT CMAKE_VERSION VERSION_LESS 3.4.0)
    set(LANG CXX)
endif()
project(MagnumVrUiGallery ${LANG})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/modules/")

find_package(Magnum REQUIRED
    MeshTools
    Primitives
    Sdl2Application
    Shaders
    Trade)
find_package(MagnumIntegration REQUIRED Ovr)
find_package(MagnumExtras REQUIRED Ui)
find_package(Leap REQUIRED)

add_executable(magnum-vr-ui-gallery
    src/VrGallery.cpp)
target_link_libraries(magnum-vr-ui-gallery PRIVATE
    MagnumExtras::Ui
    Magnum::Application
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::Shaders
    Magnum::Trade
    MagnumIntegration::Ovr
    Leap::Leap)
install(TARGETS magnum-vr-ui-gallery DESTINATION bin)
