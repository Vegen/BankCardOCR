//
// Created by huweijian on 2019/10/16.
//

#include "BitmapMatUtils.h"
#include <android/bitmap.h>

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