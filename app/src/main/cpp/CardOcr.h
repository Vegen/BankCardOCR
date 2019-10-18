//
// Created by huweijian on 2019/10/17.
//

#include "opencv2/opencv.hpp"
#include <vector>

#ifndef BANKCARDOCR_CARDOCR_H
#define BANKCARDOCR_CARDOCR_H

using namespace cv;
using namespace std;

namespace ocr {
    /**
     * 在图片中找到银行卡区域
     * @param mat 图片
     * @param rect 存放银行卡区域
     * @return 是否成功
     */
    int find_card_area(const Mat &mat, Rect &card_area);

    /**
     * 在银行卡区域找到卡号区域
     * @param mat 图片
     * @param card_area 存放卡号区域
     * @return 是否成功
     */
    int find_card_number_area(const Mat &mat, Rect &card_area);

    /**
     * 在卡号区域中找到卡号每个数字的集合
     * @param mat 图片
     * @param numbers 存放卡号每个数字的集合
     * @return 是否成功
     */
    int find_card_numbers(const Mat &mat, vector<Mat> numbers);

    /**
     * 字符串进行粘连处理
     * @param mat
     * @return 粘连的那一列
     */
    int find_split_number(const Mat &mat);

}


#endif //BANKCARDOCR_CARDOCR_H
