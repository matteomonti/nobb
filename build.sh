#!/bin/bash

red=`tput setaf 1`
reset=`tput sgr0`

# $1 debug or release
build_type=$1
compiler=$(echo ${CXX##*/})
number_of_build_workers=8

if [ "$build_type" == "" ]; then
  rm -rf build_release
  mkdir -p build_release
  cd build_release
  cmake .. -DCMAKE_BUILD_TYPE="Release"
  cd ..

  rm -rf build_debug
  mkdir -p build_debug
  cd build_debug
  cmake .. -DCMAKE_BUILD_TYPE="Debug"
  cd ..

  cd build_debug   && cmake --build . --target install -- -j${number_of_build_workers} && cd ..
  cd build_release && cmake --build . --target install -- -j${number_of_build_workers} && cd ..


elif [ "$build_type" == "Release" ] || [ "$build_type" == "release" ]; then
  echo ${green}"Building Release project"${reset}
  build_type=Release
  rm -rf build_release
  mkdir -p build_release
  cd build_release

  cmake .. -DCMAKE_BUILD_TYPE=$build_type
  cmake --build . --target install -- -j8
  cd ..

elif [ "$build_type" == "Debug" ] || [ "$build_type" == "debug" ]; then
  echo ${green}"Building Debug project"${reset}
  build_type=Debug
  rm -rf build_debug
  mkdir -p build_debug
  cd build_debug

  cmake .. -DCMAKE_BUILD_TYPE=$build_type
  cmake --build . --target install -- -j8
  cd ..

else
  echo ${red}"Unknown build type - Allowed only [Debug, Release]"${reset}
  exit 1

fi
