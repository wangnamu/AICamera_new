# AICamera_new
Recompile the library with caffe2 in pytorch1.0 and re-implement the AICamera example provided by caffe2 officially.

## caffe2 in pytorch1.0 compile and run on android

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
        jint h, jint w, jbyteArray Y, jbyteArray U, jbyteArray V,
        jint rowStride, jint pixelStride,
        jboolean infer_HWC) {
    if (!_predictor) {
        return env->NewStringUTF("Loading...");
    }
    jsize Y_len = env->GetArrayLength(Y);
    jbyte * Y_data = env->GetByteArrayElements(Y, 0);
    assert(Y_len <= MAX_DATA_SIZE);
    jsize U_len = env->GetArrayLength(U);
    jbyte * U_data = env->GetByteArrayElements(U, 0);
    assert(U_len <= MAX_DATA_SIZE);
    jsize V_len = env->GetArrayLength(V);
    jbyte * V_data = env->GetByteArrayElements(V, 0);
    assert(V_len <= MAX_DATA_SIZE);

#define min(a,b) ((a) > (b)) ? (b) : (a)
#define max(a,b) ((a) > (b)) ? (a) : (b)

    auto h_offset = max(0, (h - IMG_H) / 2);
    auto w_offset = max(0, (w - IMG_W) / 2);

    auto iter_h = IMG_H;
    auto iter_w = IMG_W;
    if (h < IMG_H) {
        iter_h = h;
    }
    if (w < IMG_W) {
        iter_w = w;
    }

    for (auto i = 0; i < iter_h; ++i) {
        jbyte* Y_row = &Y_data[(h_offset + i) * w];
        jbyte* U_row = &U_data[(h_offset + i) / 2 * rowStride];
        jbyte* V_row = &V_data[(h_offset + i) / 2 * rowStride];
        for (auto j = 0; j < iter_w; ++j) {
            // Tested on Pixel and S7.
            char y = Y_row[w_offset + j];
            char u = U_row[pixelStride * ((w_offset+j)/pixelStride)];
            char v = V_row[pixelStride * ((w_offset+j)/pixelStride)];

            float b_mean = 104.00698793f;
            float g_mean = 116.66876762f;
            float r_mean = 122.67891434f;

            auto b_i = 0 * IMG_H * IMG_W + j * IMG_W + i;
            auto g_i = 1 * IMG_H * IMG_W + j * IMG_W + i;
            auto r_i = 2 * IMG_H * IMG_W + j * IMG_W + i;

            if (infer_HWC) {
                b_i = (j * IMG_W + i) * IMG_C;
                g_i = (j * IMG_W + i) * IMG_C + 1;
                r_i = (j * IMG_W + i) * IMG_C + 2;
            }
/*
  R = Y + 1.402 (V-128)
  G = Y - 0.34414 (U-128) - 0.71414 (V-128)
  B = Y + 1.772 (U-V)
 */
            input_data[r_i] = -r_mean + (float) ((float) min(255., max(0., (float) (y + 1.402 * (v - 128)))));
            input_data[g_i] = -g_mean + (float) ((float) min(255., max(0., (float) (y - 0.34414 * (u - 128) - 0.71414 * (v - 128)))));
            input_data[b_i] = -b_mean + (float) ((float) min(255., max(0., (float) (y + 1.772 * (u - v)))));

        }
    }

    caffe2::TensorCPU input = caffe2::Tensor(1,caffe2::DeviceType::CPU);

    if (infer_HWC) {
        input.Resize(std::vector<int>({IMG_H, IMG_W, IMG_C}));
    } else {
        input.Resize(std::vector<int>({1, IMG_C, IMG_H, IMG_W}));
    }



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
unzip libs.zip in the root directory to get the static library
create a folder named armeabi-v7a in AICamera/app/src/main/jni
copy the static librarys to AICamera/app/src/main/jni/armeabi-v7a
open AICamera project with android studio
```




