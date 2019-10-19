#### 前言
在日常的开发中，我们有时会遇到添加银行卡的需求，这时候，产品可能会让你仿一下支付宝之类的相机扫描识别银行卡号。很多时候，做这样的需求会去找找稳定的第三方，本文通过 OpenCV 结合识别的需求带你分析如何实现银行卡号的识别。由于作者技术有限，本文仅从如何做到识别的思路上介绍，文中例子不适用于实际开发，也不是所有银行卡都能识别，但希望读者可以在实现的思路上给予一些启发，以及更深入熟悉 OpenCV 的组合使用。

#### 1. 银行卡识别思路分析

##### 1.1 银行卡一般具有的特征

银行卡一般会有 银行、卡号、银联标识等等，主要的是卡号区域，大多数银行卡卡号都是在下方显示的。那么，在检索一张图片的时候，要首先找到卡在哪里，卡一般是长方形，所以我们在背景色不非常接近下可以找到银行卡的轮廓。

##### 1.2 总体实现思路和步骤

> 在图片中找到银行卡区域 --> 在银行卡区域找到卡号区域 --> 在卡号区域中找到卡号每个数字的集合  --> 识别数字

#### 2. 在图片中找到银行卡区域

##### 2.1 实现思路和步骤

1. 高斯模糊降噪处理
2. 边缘梯度增强
3. 取增强绝对值
4. 合并 Mat
5. 转灰度
6. 二值化
7. 从二值图像中检索轮廓
8. 从集合中找合适的轮廓

##### 2.2 具体实现

```
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
    //imwrite("/storage/emulated/0/ocr/find_card_area_gradeMat.jpg", gradeMat);
    // 5. 转灰度
    Mat grayMat;
    cvtColor(gradeMat, grayMat, COLOR_BGRA2GRAY);
    //imwrite("/storage/emulated/0/ocr/find_card_area_grayMat.jpg", grayMat);

    // 6. 二值化
    // threshold( InputArray src, OutputArray dst, double thresh, double maxval, int type )
    // thresh：设定的阈值
    // maxval；当灰度值大于（或小于）阈值时将该灰度值赋成的值
    // type：当前二值化的方式 THRESH_BINARY 大于阈值的部分被置为255，小于部分被置为0
    // 此处其实是将 白变成黑，黑变成白
    Mat binaryMat;
    threshold(grayMat, binaryMat, 40, 255, THRESH_BINARY);

    //imwrite("/storage/emulated/0/ocr/find_card_area_binaryMat.jpg", binaryMat);

    // 7. 从二值图像中检索轮廓
    // findContours( InputOutputArray image, OutputArrayOfArrays contours, int mode, int method, Point offset = Point())
    // contours：双重向量，向量内每个元素保存了一组由连续的 Point 点构成的点的集合的向量，每一组Point点集就是一个轮廓
    // mode：定义轮廓的检索模式，其中 RETR_EXTERNAL 表示只检测最外围轮廓，包含在外围轮廓内的内围轮廓被忽略
    // method：定义轮廓的近似方法，其中 CHAIN_APPROX_SIMPLE 表示 仅保存轮廓的拐点信息，把所有轮廓拐点处的点保存入contours 向量内，拐点与拐点之间直线段上的信息点不予保留
    vector<vector<Point> > contours;
    findContours(binaryMat, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    LOGE("find_card_area contours.size():%d", contours.size());
    // 8. 从集合中找合适的轮廓
    for (int i = 0; i < contours.size(); i++) {
        // boundingRect 函数计算并返回指定点集或非零像素的灰度图像
        Rect rect = boundingRect(contours[i]);
        //LOGE("从集合中找合适的轮廓 i=%d  rect.width=%d  mat.cols=%d", i, rect.width, mat.cols);
        if (rect.width > mat.cols / 2 /*&& rect.width != mat.cols*/ && rect.height > mat.rows / 2) {
            // 银行卡区域的宽高必须大于图片的一半
            LOGE("find_card_area 哈哈哈，找到啦");
            card_area = rect;
            break;
        }
    }
    //LOGE("card_area h:%d", card_area.height);
    // 9. 释放资源
    blurMat.release();
    grade_x.release();
    grade_y.release();
    abs_grade_x.release();
    abs_grade_y.release();
    gradeMat.release();
    binaryMat.release();

    // 没有找到合适的轮廓 返回失败错误码
    if (card_area.empty()) {
        LOGE("find_card_area 啊啊啊，找不到");
        return -1;
    }

    return 0;
}
```

#### 3. 在银行卡区域找到卡号区域

##### 3.1 实现思路和步骤

此处仅做一个粗略的位置计算，实际开发时，一般和相机搭配扫描一个框的内容，也可以按框的比例来计算。

此方法有些银行卡会有误差，有误差时，实际开发应该有输入框给用户手动补充识别失败的数字。

卡号区域 起点 (x, y) = (银行卡区域宽度的 1/12, 银行卡区域高度的 1/2)

卡号区域的宽高 width = 银行卡区域宽度的 5/6, height = 银行卡区域高度的 1/4

##### 3.2 具体实现

```
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
```

#### 4. 在卡号区域中找到卡号每个数字的集合

##### 4.1 实现思路和步骤

1. 转灰度图
2. 二值化
3. 降噪过滤
4. 取反 黑变白，白变黑
5. 从二值图像中检索轮廓，黑色的背景，白色的数字，可以检测出噪点
6. 将高度小于我们设定的最小值的轮廓区域过滤掉
7. 过滤掉再合起来再取反恢复
8. 过滤后再次检索轮廓
9. 将轮廓提取成矩形轮廓集合
10. 对矩形轮廓集合进行排序
11. 裁剪出单个数字，保存数字


##### 4.2 具体实现

```
/**
 * 在卡号区域中找到卡号每个数字的集合
 * @param mat 图片
 * @param numbers 存放卡号每个数字的集合
 * @return 是否成功
 */
int ocr::find_card_numbers(const Mat &mat, vector<Mat> &numbers) {
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

    LOGE("find_card_numbers 1 contours.size():%d", contours.size());

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
    // 冒泡排序
    LOGE("contours.size()=%d", contours.size());
    if (contours.size() <= 0) return -1;
    for (int i = 0; i < contours.size() - 1; ++i) {
        for (int j = 0; j < contours.size() - i - 1; ++j) {
            if (rects[j].x > rects[j + 1].x) {
                swap(rects[j], rects[j + 1]);
            }
        }
    }

    // 11. 裁剪出单个数字，保存数字
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
            LOGE("保存数字图片:%d", i);
            char name[50];
            sprintf(name, "/storage/emulated/0/CardOCR/number_%d.jpg", i);
            imwrite(name, number);
        }
    }

    LOGE("numbers.size:%d", numbers.size());
    // 释放资源
    grayMat.release();
    binaryMat.release();
    kernel.release();
    binaryNotMat.release();
    contoursMat.release();
    return 0;
}
```

##### 4.3 字符串进行粘连处理

```
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
```

#### 5. 原图和识别效果

原图(*图片来源网络，侵删*)

![原图](https://user-gold-cdn.xitu.io/2019/10/19/16de3d7eb4145db1?w=960&h=542&f=jpeg&s=45646)

识别效果

![识别效果](https://user-gold-cdn.xitu.io/2019/10/19/16de3de8567d3370?w=1080&h=3165&f=jpeg&s=262867)


![识别效果](https://user-gold-cdn.xitu.io/2019/10/19/16de3df6f44d15b5?w=265&h=56&f=png&s=2535)

#### 后话

由于时间等原因，还需对识别出来的单个数字进行样本对比，识别出最相似的数字。作者也没有足够的样本，所以此步骤留到后续完善，但本文的初衷是为了分析识别思路和如何使用 OpenCV 进行实现，因此，读者也可结合我的基础上进行完善，本文完整的源码地址是: [https://github.com/Vegen/BankCardOCR](https://github.com/Vegen/BankCardOCR) ,欢迎 star 和 交流。
