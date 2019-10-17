#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
#include <android/log.h>

#define TAG "JNI_TAG"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

using namespace cv;
using namespace std;

extern "C" {
// bitmap 转成 Mat
void bitmap2Mat(JNIEnv *env, Mat &mat, jobject bitmap);
// mat 转成 Bitmap
void mat2Bitmap(JNIEnv *env, Mat mat, jobject bitmap);
}

// 转灰度图
JNIEXPORT jint JNICALL
Java_com_vegen_bankcardocr_BandCardOcrUtil_kt_test(JNIEnv *env, jobject instance, jobject bitmap) {
    return 0;
}


JNIEXPORT void bitmap2Mat(JNIEnv *env, Mat &mat, jobject bitmap) {
    // Mat 里面有个 type ： CV_8UC4 刚好对上我们的 Bitmap 中 ARGB_8888 , CV_8UC2 刚好对象我们的 Bitmap 中 RGB_565
    // 1. 获取 bitmap 信息
    AndroidBitmapInfo info;
    void *pixels;
    AndroidBitmap_getInfo(env, bitmap, &info);

    // 锁定 Bitmap 画布
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    // 指定 mat 的宽高和type  BGRA
    mat.create(info.height, info.width, CV_8UC4);

    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        // 对应的 mat 应该是  CV_8UC4
        Mat temp(info.height, info.width, CV_8UC4, pixels);
        // 把数据 temp 复制到 mat 里面
        temp.copyTo(mat);
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {
        // 对应的 mat 应该是  CV_8UC2
        Mat temp(info.height, info.width, CV_8UC2, pixels);
        // mat 是 CV_8UC4 ，CV_8UC2 -> CV_8UC4
        cvtColor(temp, mat, COLOR_BGR5652BGRA);
    }
    // todo 其他要自己去转

    // 解锁 Bitmap 画布
    AndroidBitmap_unlockPixels(env, bitmap);
}

JNIEXPORT void mat2Bitmap(JNIEnv *env, Mat mat, jobject bitmap) {
    // 1. 获取 bitmap 信息
    AndroidBitmapInfo info;
    void *pixels;
    AndroidBitmap_getInfo(env, bitmap, &info);

    // 锁定 Bitmap 画布
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {// C4
        LOGE("ANDROID_BITMAP_FORMAT_RGBA_8888");
        Mat temp(info.height, info.width, CV_8UC4, pixels);
        if (mat.type() == CV_8UC4) {
            LOGE("CV_8UC4");
            mat.copyTo(temp);
        } else if (mat.type() == CV_8UC2) {
            LOGE("CV_8UC2");
            cvtColor(mat, temp, COLOR_BGR5652BGRA);
        } else if (mat.type() == CV_8UC1) {// 灰度 mat
            LOGE("CV_8UC1");
            cvtColor(mat, temp, COLOR_GRAY2BGRA);
        } else if (mat.type() == CV_8UC3) {
            LOGE("CV_8UC3");
            cvtColor(mat, temp, COLOR_BGR2BGRA);
        }
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {// C2
        LOGE("ANDROID_BITMAP_FORMAT_RGB_565");
        Mat temp(info.height, info.width, CV_8UC2, pixels);
        if (mat.type() == CV_8UC4) {
            cvtColor(mat, temp, COLOR_BGRA2BGR565);
        } else if (mat.type() == CV_8UC2) {
            mat.copyTo(temp);

        } else if (mat.type() == CV_8UC1) {// 灰度 mat
            cvtColor(mat, temp, COLOR_GRAY2BGR565);
        } else if (mat.type() == CV_8UC3) {
            LOGE("CV_8UC3");
            cvtColor(mat, temp, COLOR_BGR2BGR565);
        }
    }
    // todo 其他要自己去转

    // 解锁 Bitmap 画布
    AndroidBitmap_unlockPixels(env, bitmap);
}
