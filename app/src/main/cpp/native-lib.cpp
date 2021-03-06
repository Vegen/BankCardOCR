#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
#include <android/log.h>
#include "BitmapMatUtils.h"
#include "CardOcr.h"
#include <vector>

#define TAG "tagtag"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

using namespace cv;
using namespace std;

extern "C"
JNIEXPORT jstring JNICALL
Java_com_vegen_bankcardocr_BandCardOcrUtil_bandCardOcr(JNIEnv *env, jobject instance, jobject bitmap) {

    Mat mat;
    BitmapMatUtils::bitmap2mat(env, bitmap, mat);

    //  轮廓增强（梯度增强）
    /*Rect card_area;
    ocr::find_card_area(mat, card_area);
    Mat card_area_mat(mat, card_area);*/
    //LOGE("轮廓增强卡号区域 cols:%d", card_area_mat.cols);
    // 截取到卡号区域
    Rect card_number_area;
    ocr::find_card_number_area(mat, card_number_area);
    Mat card_number_mat(mat, card_number_area);
    //LOGE("卡号区域 cols:%d", card_number_mat.cols);
    // 获取数字
    vector<Mat> numbers;
    ocr::find_card_numbers(card_number_mat, numbers);
    LOGE("识别到的数字个数：%d", numbers.size());
    if (numbers.size() != 0) {
        for (int i = 0; i < numbers.size(); i++) {
            //addWeighted(numbers[i], 0.5, mat, 0.5, )
        }
    }

    return 0;
}
