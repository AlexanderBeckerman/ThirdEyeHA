cmake_minimum_required(VERSION 3.15.3)
project(gstreamer)

set(CMAKE_CXX_STANDARD 20)

# Find and include the GStreamer libraries using PkgConfig
find_package(PkgConfig REQUIRED)
pkg_search_module(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.4)
pkg_search_module(GSTREAMER_SDP REQUIRED IMPORTED_TARGET gstreamer-sdp-1.0>=1.4)
pkg_search_module(GSTREAMER_APP REQUIRED IMPORTED_TARGET gstreamer-app-1.0>=1.4)
pkg_search_module(GSTREAMER_VIDEO REQUIRED IMPORTED_TARGET gstreamer-video-1.0>=1.4)

# Add the executable and include source files
add_executable(GstSaverApp main.cpp gst_saver.cpp)

# Link GStreamer libraries to the executable
target_link_libraries(GstSaverApp
    PkgConfig::GSTREAMER
    PkgConfig::GSTREAMER_SDP
    PkgConfig::GSTREAMER_APP
    PkgConfig::GSTREAMER_VIDEO
)
