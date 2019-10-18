//
// Created by huweijian on 2019/10/17.
//

#include "CardOcr.h"
#include <vector>

using namespace std;
using namespace cv;

/**
 * 在图片中找到银行卡区域
 * @param mat 图片
 * @param rect 银行卡区域
 * @return 是否成功
 */
int ocr::find_card_area(const Mat &mat, Rect &card_area) {
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

/**
 * 在银行卡区域找到卡号区域
 * @param mat 图片
 * @param card_area 存放卡号区域
 * @return 是否成功
 */
int ocr::find_card_number_area(const Mat &mat, Rect &card_area) {
    // 此处仅做一个粗略的位置计算，实际开发时，一般和相机搭配扫描一个框的内容，也可以按框的比例来计算。
    // 此方法有些银行卡会有误差，有误差时，实际开发应该有输入框给用户手动补充识别失败的数字。
    // 卡号区域 起点 (x, y) = (银行卡区域宽度的 1/12, 银行卡区域高度的 1/2)
    // 卡号区域的宽高 width = 银行卡区域宽度的 5/6, height = 银行卡区域高度的 1/4
    card_area.x = mat.cols / 12;
    card_area.y = mat.rows / 2;
    card_area.width = mat.cols * 5 / 6;
    card_area.height = mat.rows / 4;
    return 0;
}

/**
 * 在卡号区域中找到卡号每个数字的集合
 * @param mat 图片
 * @param numbers 存放卡号每个数字的集合
 * @return 是否成功
 */
int ocr::find_card_numbers(const Mat &mat, vector<Mat> numbers) {
    // 1. 转灰度图
    Mat grayMat;
    cvtColor(mat, grayMat, COLOR_BGRA2GRAY);

    // 2. 二值化
    Mat binaryMat;
    // THRESH_BINARY 大于阈值的部分被置为255，小于部分被置为 0
    threshold(grayMat, binaryMat, 39, 255, THRESH_BINARY);

    // 3. 降噪过滤
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    // 形态学操作
    // morphologyEx( InputArray src, OutputArray dst, int op, InputArray kernel,
    //                                Point anchor = Point(-1,-1), int iterations = 1,
    //                                int borderType = BORDER_CONSTANT,
    //                                const Scalar& borderValue = morphologyDefaultBorderValue() );
    // op：操作的类型 MORPH_CLOSE 表示闭操作，先膨胀后腐蚀
    // kernel：用于膨胀操作的结构元素 如果取值为 Mat, 那么默认使用一个 3 x 3 的方形结构元素，可以使用 getStructuringElement() 来创建结构元素
    // anchor：参考点，其默认值为(-1,-1)说明位于kernel的中心位置
    // borderType：边缘类型
    // borderValue：边缘值
    morphologyEx(binaryMat, binaryMat, MORPH_CLOSE, kernel);

    // 4. 取反 黑变白，白变黑
    Mat binaryNotMat = binaryMat.clone();
    bitwise_not(binaryNotMat, binaryNotMat);

    // 5. 从二值图像中检索轮廓，黑色的背景，白色的数字，可以检测出噪点
    vector<vector<Point> > contours;
    findContours(binaryNotMat, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // 6. 将高度小于我们设定的最小值的轮廓区域过滤掉
    int mat_area = mat.rows * mat.cols;
    int min_h = mat.rows / 4;
    for (int i = 0; i < contours.size(); i++) {
        // 所有轮廓进行检测过滤
        Rect rect = boundingRect(contours[i]);
        // 面积太小的噪点过滤掉
        int area = rect.width * rect.height;
        if (area < mat_area / 200) {
            // 小面积轮廓填充为 白色背景
            drawContours(binaryMat, contours, i, Scalar(255), -1);
        } else if (rect.height < min_h) {
            // 小面积轮廓填充为 白色背景
            drawContours(binaryMat, contours, i, Scalar(255), -1);
        }
    }

    // 7. 过滤掉再合起来再取反恢复
    binaryMat.copyTo(binaryNotMat);
    bitwise_not(binaryNotMat, binaryNotMat);

    // 8. 过滤后再次检索轮廓
    contours.clear();
    findContours(binaryNotMat, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // 9. 将轮廓提取成矩形轮廓集合，找到最小宽度轮廓
    Rect rects[contours.size()];
    // 白色的图片，单颜色
    Mat contoursMat(binaryMat.size(), CV_8UC1, Scalar(255));
    // min_w 存放最小宽度轮廓
    int min_w = mat.cols;
    for (int i = 0; i < contours.size(); i++) {
        rects[i] = boundingRect(contours[i]);
        drawContours(contoursMat, contours, i, Scalar(0), 1);
        min_w = min(rects[i].width, min_w);
    }

    // 10. 对矩形轮廓集合进行排序
    // todo 冒泡排序
    /*for (int i = 0; i < contours.size() - 1; ++i) {
        for (int j = 0; j < contours.size() - i - 1; ++j) {
            if (rects[j].x > rects[j + 1].x) {
                *//*Rect temp = rects[j + 1];
                rects[j + 1] = rects[j];
                rects[j] = temp;*//*
                //swap(rects[j], rects[j + 1]);
            }
        }
    }*/

    // 12. 裁剪出单个数字，保存数字
    numbers.clear();
    for (int i = 0; i < contours.size(); i++) {
        // >= 最小宽度的两倍，是粘连的数字
        if (rects[i].width >= min_h * 2) {
            // 处理粘连字符
            Mat mat(contoursMat, rects[i]);
            int cols_pos = find_split_number(mat);
            // 左右两个数字都存进去
            Rect rect_left(0, 0, cols_pos - 1, mat.rows);
            numbers.push_back(Mat(mat, rect_left));
            Rect rect_right(cols_pos, 0, mat.cols, mat.rows);
            numbers.push_back(Mat(mat, rect_right));
        } else {
            Mat number(contoursMat, rects[i]);
            numbers.push_back(number);
            // 保存数字图片
            //char name[50];
            //sprintf(name, "/storage/emulated/0/ocr/card_number_%d.jpg", i);
            //imwrite(name, number);
        }
    }

    // 释放资源
    grayMat.release();
    binaryMat.release();
    kernel.release();
    binaryNotMat.release();
    contoursMat.release();
    return 0;
}

/**
 * 字符串进行粘连处理
 * @param mat
 * @return 粘连的那一列
 */
int ocr::find_split_number(const Mat &mat) {
    // 对中心位置的左右 1/4 扫描，记录最少的黑色像素点的这一列的位置，就当做字符串的粘连位置
    int mx = mat.cols / 2;
    int height = mat.rows;
    // 围绕中心左右扫描 1/4
    int start_x = mx - mx / 2;
    int end_x = mx + mx / 2;
    // 字符的粘连位置
    int cols_pos = mx;
    // 获取像素子
    int c = 0;
    // 最小的像素值
    int min_h_p = mat.rows;

    for (int col = start_x; col < end_x; ++col) {
        int total = 0;
        for (int row = 0; row < height; ++row) {
            c = mat.at<Vec3b>(row, col)[0];// 单通道
            if (c == 0) {
                total++;
            }
        }
        if (total < min_h_p) {
            min_h_p = total;
            cols_pos = col;
        }
    }
    return cols_pos;
}