cmake_minimum_required(VERSION 2.8)
project( GalaxyCameraOpencv )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )


aux_source_directory(inc DahengUSB3CameraCV_INCLUDE_DIRS )
aux_source_directory(src DahengUSB3CameraCV_SRC_DIRS )
include_directories(inc)

add_library(DahengUSB3CameraCV ${DahengUSB3CameraCV_SRC_DIRS})
target_link_libraries( DahengUSB3CameraCV ${OpenCV_LIBS} gxiapi )


add_executable( camera_show test/cv_imshow.cpp )
target_link_libraries( camera_show DahengUSB3CameraCV )

add_executable( gx_test test/gx_test.cpp )
target_link_libraries( gx_test gxiapi pthread )