//
// Created by huweijian on 2019/10/17.
//

#include "CardOcr.h"
#include <vector>

using namespace std;

/**
 * 在图片中找到银行卡区域
 * @param mat 图片
 * @param rect 银行卡区域
 * @return 是否成功
 */
int orc::find_card_area(const Mat &mat, Rect &card_area) {
    // 1. 高斯模糊降噪处理
    Mat blurMat;
    GaussianBlur(mat, blurMat, Size(5, 5), BORDER_DEFAULT, BORDER_DEFAULT);

    // 2. 边缘梯度增强
    // Scharr( InputArray src, OutputArray dst, int ddepth, int dx, int dy, double scale = 1, double delta = 0, int borderType = BORDER_DEFAULT )
    // ddepth：输出图像的深度
    // dx：x方向上的差分阶数
    // dy：y方向上的差分阶数

    // 对 x 增强
    Mat grade_x;
    Scharr(blurMat, grade_x, CV_32F, 1, 0);
    // 对 y 增强
    Mat grade_y;
    Scharr(blurMat, grade_y, CV_32F, 0, 1);

    // 3. 取增强绝对值
    Mat abs_grade_x;
    convertScaleAbs(grade_x, abs_grade_x);
    Mat abs_grade_y;
    convertScaleAbs(grade_y, abs_grade_y);

    // 4. 合并 Mat
    Mat gradeMat;
    addWeighted(abs_grade_x, 0.5, abs_grade_y, 0.5, 0, gradeMat);

    // 5. 转灰度
    Mat grayMat;
    cvtColor(gradeMat, gradeMat, COLOR_RGBA2BGRA);

    // 6. 二值化
    // threshold( InputArray src, OutputArray dst, double thresh, double maxval, int type )
    // thresh：设定的阈值
    // maxval；当灰度值大于（或小于）阈值时将该灰度值赋成的值
    // type：当前二值化的方式 THRESH_BINARY 大于阈值的部分被置为255，小于部分被置为0
    // 此处其实是将 白变成黑，黑变成白
    Mat binaryMat;
    threshold(grayMat, binaryMat, 40, 255, THRESH_BINARY);

    // 7. 从二值图像中检索轮廓
    // findContours( InputOutputArray image, OutputArrayOfArrays contours, int mode, int method, Point offset = Point())
    // contours：双重向量，向量内每个元素保存了一组由连续的 Point 点构成的点的集合的向量，每一组Point点集就是一个轮廓
    // mode：定义轮廓的检索模式，其中 RETR_EXTERNAL 表示只检测最外围轮廓，包含在外围轮廓内的内围轮廓被忽略
    // method：定义轮廓的近似方法，其中 CHAIN_APPROX_SIMPLE 表示 仅保存轮廓的拐点信息，把所有轮廓拐点处的点保存入contours 向量内，拐点与拐点之间直线段上的信息点不予保留
    vector<vector<Point> > contours;
    findContours(binaryMat, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // 8. 从集合中找合适的轮廓
    for (int i = 0; i < contours.size(); i++) {
        // boundingRect 函数计算并返回指定点集或非零像素的灰度图像
        Rect rect = boundingRect(contours[i]);
        if (rect.width > mat.cols / 2 && rect.width != mat.cols && rect.height > mat.rows / 2) {
            // 银行卡区域的宽高必须大于图片的一半
            card_area = rect;
            break;
        }
    }

    // 9. 释放资源
    blurMat.release();
    grade_x.release();
    grade_y.release();
    abs_grade_x.release();
    abs_grade_y.release();
    gradeMat.release();
    binaryMat.release();

    // 没有找到合适的轮廓 返回失败错误码
    if (card_area.empty()) return -1;

    return 0;
}