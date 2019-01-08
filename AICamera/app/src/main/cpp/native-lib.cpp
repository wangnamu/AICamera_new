#include <jni.h>
#include <string>
#include <algorithm>
#define PROTOBUF_USE_DLLS 1
#define CAFFE2_USE_LITE_PROTO 1
#include <caffe2/predictor/predictor.h>
#include <caffe2/core/operator.h>
#include <caffe2/core/timer.h>

#include "caffe2/core/init.h"

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include "classes.h"
#include "libyuv.h"

#define IMG_H 227
#define IMG_W 227
#define IMG_C 3
#define MAX_DATA_SIZE IMG_H * IMG_W * IMG_C
#define alog(...) __android_log_print(ANDROID_LOG_ERROR, "AICamera", __VA_ARGS__);

static caffe2::NetDef _initNet, _predictNet;
static caffe2::Predictor *_predictor;
static float input_data[MAX_DATA_SIZE];
static caffe2::Workspace ws;

// A function to load the NetDefs from protobufs.
void loadToNetDef(AAssetManager* mgr, caffe2::NetDef* net, const char *filename) {
    AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
    assert(asset != nullptr);
    const void *data = AAsset_getBuffer(asset);
    assert(data != nullptr);
    off_t len = AAsset_getLength(asset);
    assert(len != 0);
    if (!net->ParseFromArray(data, len)) {
        alog("Couldn't parse net from data.\n");
    }
    AAsset_close(asset);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ufo_aicamera_MainActivity_initCaffe2(JNIEnv *env, jobject /* this */, jobject assetManager) {

    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    alog("Attempting to load protobuf netdefs...");
    loadToNetDef(mgr, &_initNet,   "squeeze_init_net.pb");
    loadToNetDef(mgr, &_predictNet,"squeeze_predict_net.pb");
    alog("done.");
    alog("Instantiating predictor...");
    _predictor = new caffe2::Predictor(_initNet, _predictNet);
    alog("done.")

}

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