# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/asmusrise/Documents/Rasperry_pico/GrapperCode/build/_deps/picotool-src"
  "/home/asmusrise/Documents/Rasperry_pico/GrapperCode/build/_deps/picotool-build"
  "/home/asmusrise/Documents/Rasperry_pico/GrapperCode/build/_deps"
  "/home/asmusrise/Documents/Rasperry_pico/GrapperCode/build/picotool/tmp"
  "/home/asmusrise/Documents/Rasperry_pico/GrapperCode/build/picotool/src/picotoolBuild-stamp"
  "/home/asmusrise/Documents/Rasperry_pico/GrapperCode/build/picotool/src"
  "/home/asmusrise/Documents/Rasperry_pico/GrapperCode/build/picotool/src/picotoolBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/asmusrise/Documents/Rasperry_pico/GrapperCode/build/picotool/src/picotoolBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/asmusrise/Documents/Rasperry_pico/GrapperCode/build/picotool/src/picotoolBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
