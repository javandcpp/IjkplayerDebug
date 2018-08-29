# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(include-ffmpegheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/ffmpeg)
include_directories(${include-ffmpegheaders})

set(include-clheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/CL)
include_directories(${include-clheaders})

set(include-opensslheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/openssl)
include_directories(${include-opensslheaders})

set(include-ijkjheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/ijkj4a)
include_directories(${include-ijkjheaders})

set(include-ijkplayerheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/ijkplayer)
include_directories(${include-ijkplayerheaders})

set(include-yuvheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/libyuv)
include_directories(${include-yuvheaders})


set(include-ijksdlheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/ijksdl)
include_directories(${include-ijksdlheaders})

set(include-ijksoundtouchheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/ijksoundtouch)
include_directories(${include-ijksoundtouchheaders})


set(include-ijksoundtouchotherheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/ijksoundtouch/include)
include_directories(${include-ijksoundtouchotherheaders})

set(include-ijkprofheaders ${CMAKE_SOURCE_DIR}/src/main/cpp/ijkprof)
include_directories(${include-ijkprofheaders})



file(GLOB native_ijkj_srcs "src/main/cpp/ijkj4a/*.c")

file(GLOB native_ijkj_android_pipeline_srcs "src/main/cpp/ijkj4a/j4a/*.c")
file(GLOB native_ijkj_android_srcs "src/main/cpp/ijkplayer/android/*.c")

file(GLOB native_player_srcs "src/main/cpp/ijkplayer/*.c")
file(GLOB native_ijkavformat_srcs "src/main/cpp/ijkplayer/ijkavformat/*.c")
file(GLOB native_ijkavutil_srcs "src/main/cpp/ijkplayer/ijkavutil/*.c")
file(GLOB native_ijkavutilcpp_srcs "src/main/cpp/ijkplayer/ijkavutil/*.cpp")
file(GLOB native_ijkplayer_pipeline_srcs "src/main/cpp/ijkplayer/android/pipeline/*.c")
file(GLOB native_pipeline_srcs "src/main/cpp/ijkplayer/pipeline/*.c")

file(GLOB native_soundtouch_srcs "src/main/cpp/ijksoundtouch/*.c")
file(GLOB native_soundtouchother_srcs "src/main/cpp/ijksoundtouch/*.cpp")
file(GLOB native_soundtouchotherother_srcs "src/main/cpp/ijksoundtouch/source/SoundTouch/*.cpp")
file(GLOB native_ijkprofsrcs "src/main/cpp/ijkprof/*.c")

file(GLOB ffmpeg_srcs "src/main/cpp/ffmpeg/*.c")
file(GLOB ffmpeg_compat "src/main/cpp/compat/*.c")
file(GLOB ffmpeg_avcodec "src/main/cpp/libavcodec/*.c")







add_library(ffmpeg SHARED IMPORTED)
set_target_properties(ffmpeg
                      PROPERTIES
                      IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libijkffmpeg.so)

add_library(sdl SHARED IMPORTED)
set_target_properties(sdl
                      PROPERTIES
                      IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libijksdl.so)

add_library(ijkplayer SHARED IMPORTED)
set_target_properties(ijkplayer
                      PROPERTIES
                      IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/main/jniLibs/armeabi-v7a/libijkplayer.so)

add_library( # Sets the name of the library.
             ijkplayerMedia

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).

             ${native_ijkj_srcs}
             ${native_player_srcs}
             ${native_ijkj_android_srcs}
             ${native_ijkavformat_srcs}
             ${native_ijkavutil_srcs}
             ${native_ijkplayer_pipeline_srcs}
             ${native_pipeline_srcs}
             ${native_soundtouch_srcs}
             ${native_soundtouchother_srcs}
             ${native_soundtouchotherother_srcs}
             ${native_ijkavutilcpp_srcs}
             ${native_ijkprofsrcs}

             ${ffmpeg_srcs}
             ${ffmpeg_compat}
             ${ffmpeg_avcodec}
             )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       ijkplayerMedia

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} sdl log android)