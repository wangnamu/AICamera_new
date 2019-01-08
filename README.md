# AICamera_new
Recompile the library with caffe2 in pytorch stable(1.0) and re-implement the AICamera example provided by caffe2 officially.

## caffe2 in pytorch stable(1.0) compile and run on android

### Compile the android static library inside caffe2

*  git clone https://github.com/pytorch/pytorch.git

*  go to pytorch/scripts

*  add the following code to the file build_android.sh (otherwise the compiled file will be very large)

```
CMAKE_ARGS+=("-DCMAKE_CXX_FLAGS_RELEASE=-g0")
```

* run build_android.sh

* waiting for compilation to complete

* get all the libraries in pytorch/build_android/lib

```
libc10.a
libcaffe2_detectron_ops.so
libcaffe2_protos.a
libcaffe2.a
libclog.a
libcpuinfo.a
libnnpack_reference_layers.a
libnnpack.a
libonnx_proto.a
libonnx.a
libonnxifi_dummy.so
libonnxifi_loader.a
libprotobuf-lite.a
libprotobuf.a
libpthreadpool.a
libqnnpack.a
```

### Add the library to the android project

* copy the above file to android project app/src/main/jni/armeabi-v7a

* copy the following folder to android project app/src/main/cpp and delete files other than header files

```
pytorch/caffe2
pytorch/aten/src/ATen
pytorch/c10
pytorch/third_party/protobuf/src/google/protobuf 
```
* then perform the following operations

```
copy pytorch/build_android/c10/macros/cmake_macros.h to app/src/main/cpp/c10/macros

copy header file inside pytorch/build_android/caffe2/proto to app/src/main/cpp/caffe2/proto

```


* CMakeLists.txt like this

```
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
        ${log-lib}
        ${android-lib})
```

### CPP code

```CPP
float avg_fps = 0.0;
float total_fps = 0.0;
int iters_fps = 10;

extern "C"
JNIEXPORT jstring JNICALL
Java_com_ufo_aicamera_MainActivity_predFromCaffe2(
        JNIEnv *env,
        jobject /* this */,
        jbyteArray y, jbyteArray u, jbyteArray v,
        jint width, jint height, jint y_row_stride,
        jint uv_row_stride, jint uv_pixel_stride,
        jint scale_width, jint scale_height, jint degree) {

    if (!_predictor) {
        return env->NewStringUTF("Loading...");
    }


    jbyte *const y_buff = env->GetByteArrayElements(y, 0);
    jbyte *const u_buff = env->GetByteArrayElements(u, 0);
    jbyte *const v_buff = env->GetByteArrayElements(v, 0);

    uint8_t* argb = new uint8_t[scale_width * scale_height * 4];


    const int y_plane_length = width * height;
    const int uv_plane_length = y_plane_length / 4;
    const int buffer_length = y_plane_length + uv_plane_length * 2;
    std::unique_ptr<uint8_t> buffer(new uint8_t[buffer_length]);

    libyuv::Android420ToI420(
            reinterpret_cast<uint8_t *>(y_buff),
            y_row_stride,
            reinterpret_cast<uint8_t *>(u_buff),
            uv_row_stride,
            reinterpret_cast<uint8_t *>(v_buff),
            uv_row_stride,
            uv_pixel_stride,
            buffer.get(),
            width,
            buffer.get() + y_plane_length,
            width / 2,
            buffer.get() + y_plane_length + uv_plane_length,
            width / 2,
            width,
            height
    );

    const int scale_y_plane_length = scale_height * scale_width;
    const int scale_uv_plane_length = scale_y_plane_length / 4;
    const int scale_buffer_length = scale_y_plane_length + scale_uv_plane_length * 2;
    std::unique_ptr<uint8_t> scale_buffer(new uint8_t[scale_buffer_length]);

    libyuv::I420Scale(
            buffer.get(),
            width,
            buffer.get() + y_plane_length,
            width / 2,
            buffer.get() + y_plane_length + uv_plane_length,
            width / 2,
            width,
            height,
            scale_buffer.get(),
            scale_width,
            scale_buffer.get() + scale_y_plane_length,
            scale_width / 2,
            scale_buffer.get() + scale_y_plane_length + scale_uv_plane_length,
            scale_width / 2,
            scale_width,
            scale_height,
            libyuv::kFilterNone
    );

    const int rotate_y_plane_length = scale_height * scale_width;
    const int rotate_uv_plane_length = rotate_y_plane_length / 4;
    const int rotate_buffer_length = rotate_y_plane_length + rotate_uv_plane_length * 2;
    std::unique_ptr<uint8_t> rotate_buffer(new uint8_t[rotate_buffer_length]);

    libyuv::I420Rotate(
            scale_buffer.get(),
            scale_width,
            scale_buffer.get() + scale_y_plane_length,
            scale_width / 2,
            scale_buffer.get() + scale_y_plane_length + scale_uv_plane_length,
            scale_width / 2,
            rotate_buffer.get(),
            scale_height,
            rotate_buffer.get() + rotate_y_plane_length,
            scale_height / 2,
            rotate_buffer.get() + rotate_y_plane_length + rotate_uv_plane_length,
            scale_height / 2,
            scale_width,
            scale_height,
            (libyuv::RotationMode) degree
    );


    libyuv::I420ToARGB(
            rotate_buffer.get(),
            scale_width,
            rotate_buffer.get() + rotate_y_plane_length,
            scale_width / 2,
            rotate_buffer.get() + rotate_y_plane_length + rotate_uv_plane_length,
            scale_width / 2,
            argb,
            scale_width * 4,
            scale_width,
            scale_height
    );


    for (int i = 0; i < scale_width * scale_height * 4; i += 4) {
        int b = (argb[i]) & 0xFF;
        int g = (argb[i + 1]) & 0xFF;
        int r = (argb[i + 2]) & 0xFF;

        input_data[i / 4] = r;
        input_data[i / 4 + scale_width * scale_height] = g;
        input_data[i / 4 + scale_width * scale_height * 2] = b;
    }


    caffe2::TensorCPU input = caffe2::Tensor(1,caffe2::DeviceType::CPU);

    input.Resize(std::vector<int>({1, IMG_C, IMG_H, IMG_W}));


    memcpy(input.mutable_data<float>(), input_data, IMG_H * IMG_W * IMG_C * sizeof(float));
    caffe2::Predictor::TensorList input_vec{input};
    caffe2::Predictor::TensorList output_vec;
    caffe2::Timer t;
    t.Start();
    _predictor->operator()(input_vec, &output_vec);
    float fps = 1000/t.MilliSeconds();
    total_fps += fps;
    avg_fps = total_fps / iters_fps;
    total_fps -= avg_fps;


    delete[] argb;


    constexpr int k = 5;
    float max[k] = {0};
    int max_index[k] = {0};
    // Find the top-k results manually.
    if (output_vec.capacity() > 0) {
        for (auto output : output_vec) {
            for (auto i = 0; i < output.size(); ++i) {
                for (auto j = 0; j < k; ++j) {
                    if (output.template data<float>()[i] > max[j]) {
                        for (auto _j = k - 1; _j > j; --_j) {
                            max[_j - 1] = max[_j];
                            max_index[_j - 1] = max_index[_j];
                        }
                        max[j] = output.template data<float>()[i];
                        max_index[j] = i;
                        goto skip;
                    }
                }
                skip:;
            }
        }
    }
    std::ostringstream stringStream;
    stringStream << avg_fps << " FPS\n";

    for (auto j = 0; j < k; ++j) {
        stringStream << j << ": " << imagenet_classes[max_index[j]] << " - " << max[j] / 10 << "%\n";
    }

    env->ReleaseByteArrayElements(u, u_buff, JNI_ABORT);
    env->ReleaseByteArrayElements(v, v_buff, JNI_ABORT);
    env->ReleaseByteArrayElements(y, y_buff, JNI_ABORT);

    return env->NewStringUTF(stringStream.str().c_str());
}

```

### See the AICamera_new project for the complete code.

## The Example

* code based on https://github.com/caffe2/AICamera

* successfully built on Android Studio 3.2.1

* Successfully tested at Samsung Galaxy S6

* run the example
```
git clone https://github.com/wangnamu/AICamera_new.git
unzip libs.zip in the root directory to get the static libraries
create a folder named armeabi-v7a in AICamera/app/src/main/jni
copy the static libraries to AICamera/app/src/main/jni/armeabi-v7a
open AICamera project with android studio
```




