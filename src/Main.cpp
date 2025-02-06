/**
 * @Author: CsVeryLoveXieWenLi
 * @LastEditTime: 2025-02-06 19:02:22
 * @Description: 主程序
 * @Copyright (c) 2024 by CsVeryLoveXieWenLi, All Rights Reserved.
 */

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


/**
 * 易语言：识别滑块
 */
int ESliderOcr(unsigned char* bg_bytes, int blength, unsigned char* gap_bytes, int glength) {
    // 非法数据
    if (bg_bytes == nullptr || gap_bytes == nullptr || blength <= 0 || glength <= 0) return 0;

    // 解码，若小图有透明通道，直接转化全透明为黑色背景
    cv::Mat bg_image  = cv::imdecode(cv::Mat(1, blength, CV_8UC1, bg_bytes), cv::IMREAD_COLOR);
    cv::Mat gap_image = cv::imdecode(cv::Mat(1, glength, CV_8UC1, gap_bytes), cv::IMREAD_COLOR);

    // 解码图片失败
    if (bg_image.empty() || gap_image.empty()) return 0;


    // 处理大图和小图
    cv::Mat gap_image_canny, bg_image_canny;

    cv::GaussianBlur(bg_image, bg_image, cv::Size(3, 3), 0.0);
    cv::Canny(bg_image, bg_image_canny, 120.0, 220.0);

    cv::GaussianBlur(gap_image, gap_image, cv::Size(9, 9), 0.0);
    cv::Canny(gap_image, gap_image_canny, 140.0, 240.0);


    // 寻找缺口
    int    w = gap_image_canny.cols, h = gap_image_canny.rows, step = gap_image_canny.step;
    int    x1 = gap_image_canny.cols, y1 = gap_image_canny.rows, x2 = 0, y2 = 0;
    uchar* data = gap_image_canny.data;

    for (int y = 0; y < h; y++) {
        int ptr = y * step;

        for (int x = 0; x < w; x++) {
            uchar pixel = data[ptr + x];
            if (pixel == 0) continue;

            if (x < x1) x1 = x;
            if (y < y1) y1 = y;

            if (x > x2) x2 = x;
            if (y > y2) y2 = y;
        }
    }

    // 裁剪得到缺口
    cv::Mat crop_image_canny = gap_image_canny(cv::Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1));


    // 模板匹配
    cv::Mat   result;
    cv::Point point;

    // cv::imwrite("crop_image_canny.jpg", crop_image_canny);
    // cv::imwrite("bg_image_canny.jpg", bg_image_canny);

    cv::matchTemplate(bg_image_canny, crop_image_canny, result, cv::TM_CCOEFF);
    cv::minMaxLoc(result, nullptr, nullptr, nullptr, &point);


    return point.x;
}


/**
 * 易语言：设置线程数
 */
void ESliderThread(int num) { cv::setNumThreads(num); }