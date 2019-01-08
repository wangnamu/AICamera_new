# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)


add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/native-lib.cpp)

find_library(
        android-lib
        android
)

include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()

add_library(
        c10
        STATIC
        IMPORTED)

set_target_properties(
        c10
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libc10.a)


add_library(
        caffe2
        STATIC
        IMPORTED)

set_target_properties(
        caffe2
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libcaffe2.a)


add_library(
        caffe2_protos
        STATIC
        IMPORTED)

set_target_properties(
        caffe2_protos
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libcaffe2_protos.a)

add_library(
        clog
        SHARED
        IMPORTED
)
set_target_properties(
        clog
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libclog.a
)

add_library(
        cpuinfo
        STATIC
        IMPORTED)

set_target_properties(
        cpuinfo
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libcpuinfo.a)

add_library(
        NNPACK
        STATIC
        IMPORTED
)
set_target_properties(
        NNPACK
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libnnpack.a
)

add_library(
        NNPACK_REFERENCE_LAYERS
        STATIC
        IMPORTED
)
set_target_properties(
        NNPACK_REFERENCE_LAYERS
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libnnpack_reference_layers.a
)

add_library(
        ONNX
        STATIC
        IMPORTED
)
set_target_properties(
        ONNX
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libonnx.a
)

add_library(
        ONNX_PROTO
        STATIC
        IMPORTED
)
set_target_properties(
        ONNX_PROTO
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libonnx_proto.a
)



add_library(
        ONNXIFI_LOADER
        STATIC
        IMPORTED
)
set_target_properties(
        ONNXIFI_LOADER
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libonnxifi_loader.a
)


add_library(
        protobuf
        SHARED
        IMPORTED)

set_target_properties(
        protobuf
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libprotobuf.a)


add_library(
        protobuf_lite
        SHARED
        IMPORTED)

set_target_properties(
        protobuf_lite
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libprotobuf-lite.a)


add_library(
        thread_pool
        STATIC
        IMPORTED
)
set_target_properties(
        thread_pool
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libpthreadpool.a
)

add_library(
        libqnnpack
        STATIC
        IMPORTED
)
set_target_properties(
        libqnnpack
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libqnnpack.a
)

add_library(
        libyuv_static
        STATIC
        IMPORTED
)
set_target_properties(
        libyuv_static
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_LIST_DIR}/src/main/jni/${ANDROID_ABI}/libyuv_static.a
)


include_directories(src/main/cpp)


find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

target_link_libraries( # Specifies the target library.
        native-lib

        -Wl,--whole-archive
        caffe2
        -Wl,--no-whole-archive
        NNPACK
        NNPACK_REFERENCE_LAYERS
        cpuinfo
        thread_pool
        clog
        protobuf
        protobuf_lite

        ONNX
        ONNX_PROTO

        ONNXIFI_LOADER

        caffe2_protos
        c10
        libqnnpack

        libyuv_static

        cpufeatures

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${android-lib})