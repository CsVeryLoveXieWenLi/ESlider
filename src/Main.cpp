/**
 * @Author: CsVeryLoveXieWenLi
 * @LastEditTime: 2025-02-03 21:46:21
 * @Description: 主程序
 * @Copyright (c) 2024 by CsVeryLoveXieWenLi, All Rights Reserved.
 */

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


/**
 * 易语言：识别滑块
 */
int ESliderOcr(unsigned char* bg_bytes, int blength, unsigned char* gap_bytes, int glength) {
    if (bg_bytes == nullptr || gap_bytes == nullptr || blength == 0 || glength == 0) return 0;

    // 从内存中解码图片，忽略透明度通道
    // 若忽略透明度通道，全透明处则变为黑色背景
    cv::Mat bg_image  = cv::imdecode(cv::Mat(1, blength, CV_8UC1, bg_bytes), cv::IMREAD_COLOR);
    cv::Mat gap_image = cv::imdecode(cv::Mat(1, glength, CV_8UC1, gap_bytes), cv::IMREAD_COLOR);

    if (bg_image.empty() || gap_image.empty()) return 0;


    // 对大图进行高斯模糊，减少边缘信息
    // 然后进行边缘检测，获得更清晰的轮廓信息
    cv::Mat bg_image_canny;
    cv::GaussianBlur(bg_image, bg_image, cv::Size(7, 7), 0.0);

    cv::Canny(bg_image, bg_image_canny, 190.0, 300.0);


    // 对小图进行扩充，防止检测不到边缘
    // 对小图进行二值化，凸显图中的缺口
    // 然后进行边缘检测，获得更清晰的轮廓信息
    cv::Mat gap_image_canny;
    cv::copyMakeBorder(gap_image, gap_image, 10, 10, 10, 10, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
    cv::cvtColor(gap_image, gap_image, cv::COLOR_BGR2GRAY);
    cv::threshold(gap_image, gap_image, 0.0, 255.0, cv::THRESH_BINARY);

    cv::Canny(gap_image, gap_image_canny, 190.0, 300.0);


    // 对小图进行轮廓检测，提取图中的缺口
    // 寻找面积最大的轮廓，便是缺口

    // 警告，若图中缺口边缘有纯黑像素，则可能存在问题
    // 可使用别的方法定位图中缺口，这里就用这个，懒得写别的
    std::vector<std::vector<cv::Point>> gap_image_contours;
    cv::findContours(gap_image_canny, gap_image_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (gap_image_contours.empty()) return 0;

    double max_area  = 0.0;
    int    max_index = 0;

    for (int index = 0, num = gap_image_contours.size(); index < num; index++) {
        auto& contour = gap_image_contours[index];

        double area = cv::contourArea(contour);
        if (area > max_area) {
            max_area  = area;
            max_index = index;
        }
    }

    cv::Mat crop_image_canny = gap_image_canny(cv::boundingRect(gap_image_contours[max_index]));


    // 进行模板匹配，查找最优结果
    cv::Mat   result;
    cv::Point point;
    cv::matchTemplate(bg_image_canny, crop_image_canny, result, cv::TM_CCORR);
    cv::minMaxLoc(result, nullptr, nullptr, nullptr, &point);

    return point.x;
}


/**
 * 易语言：设置线程数
 */
void ESliderThread(int num) { cv::setNumThreads(num); }