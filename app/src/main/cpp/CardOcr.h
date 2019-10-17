//
// Created by huweijian on 2019/10/17.
//

#include "opencv2/opencv.hpp"

#ifndef BANKCARDOCR_CARDOCR_H
#define BANKCARDOCR_CARDOCR_H

using namespace cv;

namespace orc {
    /**
     * 在图片中找到银行卡区域
     * @param mat 图片
     * @param rect 银行卡区域
     * @return 是否成功
     */
    int find_card_area(const Mat &mat, Rect &card_area);
}


#endif //BANKCARDOCR_CARDOCR_H
