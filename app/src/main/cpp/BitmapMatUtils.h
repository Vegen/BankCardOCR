//
// Created by huweijian on 2019/10/16.
//
#include <jni.h>
#include <opencv2/opencv.hpp>

#ifndef BANKCARDOCR_BITMAPMATUTILS_H
#define BANKCARDOCR_BITMAPMATUTILS_H


using namespace cv;

class BitmapMatUtils {
public:
    // 开发项目增强，方法怎么写
    // java 中是把想要的结果返回
    // c/c++ 结果参数传递，返回值一般返回是否成功
    static int bitmap2mat(JNIEnv* env,jobject bitmap, Mat &mat);
    /*
     * mat -> bitmap
     */
    static int mat2bitmap(JNIEnv* env,jobject bitmap, Mat &mat);
};


#endif //BANKCARDOCR_BITMAPMATUTILS_H
