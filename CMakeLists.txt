cmake_minimum_required (VERSION 3.19)
project("${CMAKE_PROJECT_NAME}")
include("$ENV{TOOLCHAIN_PATH}/CMakeLists.txt.in")

add_lib("libcmd" "source/lib" ${LIB_NET} ${LIB_LOG} ${LIB_COM})
add_bin("sendcmd" "source/bin/sendcmd" libcmd ${LIB_NET} ${LIB_CFG} ${LIB_LOG} ${LIB_COM})
add_unittest(libcmd ${LIB_NET} ${LIB_LOG} ${LIB_COM})
