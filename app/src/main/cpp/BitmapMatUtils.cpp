//
// Created by huweijian on 2019/10/16.
//

#include "BitmapMatUtils.h"
#include <android/bitmap.h>
#include <android/log.h>
#define TAG "tagtag"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)


int BitmapMatUtils::bitmap2mat(JNIEnv *env, jobject bitmap, Mat &mat) {
    // 1. 锁定画布
    void *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    // 构建 mat 对象，还要判断什么颜色通道 0-255
    // 获取 Bitmap 的信息
    AndroidBitmapInfo bitmapInfo;
    AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);

    // 返回三通道 CV_8UC4-> argb CV_8UC2->rgb CV_8UC1->黑白
    Mat createMat(bitmapInfo.height, bitmapInfo.width, CV_8UC4);
    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) { // mat 里面的四颜色通道 CV_8UC4
        Mat temp(bitmapInfo.height,bitmapInfo.width,CV_8UC4 ,pixels);
        temp.copyTo(createMat);
    } else if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565) {// mat 三颜色 CV_8UC2
        Mat temp(bitmapInfo.height,bitmapInfo.width,CV_8UC2 ,pixels);
        cvtColor(temp,createMat,COLOR_BGR5652BGRA);
    }

    createMat.copyTo(mat);

    // 2. 解锁画布
    AndroidBitmap_unlockPixels(env, bitmap);

    return 0;
};

int BitmapMatUtils::mat2bitmap(JNIEnv *env, jobject bitmap, Mat &mat) {
    // 1. 获取 bitmap 信息
    AndroidBitmapInfo info;
    void *pixels;
    AndroidBitmap_getInfo(env, bitmap, &info);

    // 锁定 Bitmap 画布
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    LOGE("info.format=%d mat.type()=%d", info.format, mat.type());
    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {// C4
        Mat temp(info.height, info.width, CV_8UC4, pixels);
        if (mat.type() == CV_8UC4) {
            mat.copyTo(temp);
        } else if (mat.type() == CV_8UC2) {
            cvtColor(mat, temp, COLOR_BGR5652BGRA);
        } else if (mat.type() == CV_8UC1) {// 灰度 mat
            cvtColor(mat, temp, COLOR_GRAY2BGRA);
        }
    } else if (info.format == ANDROID_BITMAP_FORMAT_RGB_565) {// C2
        Mat temp(info.height, info.width, CV_8UC2, pixels);
        if (mat.type() == CV_8UC4) {
            cvtColor(mat, temp, COLOR_BGRA2BGR565);
        } else if (mat.type() == CV_8UC2) {
            mat.copyTo(temp);

        } else if (mat.type() == CV_8UC1) {// 灰度 mat
            cvtColor(mat, temp, COLOR_GRAY2BGR565);
        }
    }
    // todo 其他要自己去转

    // 解锁 Bitmap 画布
    AndroidBitmap_unlockPixels(env, bitmap);
    return 0;
}