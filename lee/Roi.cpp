#include "Roi.h"
#include <opencv2/opencv.hpp>

/**
 * @brief 생성자. points, mask 기본 세팅
 * @param is_right 우측 roi 영역이라면 true
 */
Roi::Roi(bool is_right) {
    this->is_right = is_right;

    if (this->is_right) {
        this->points.emplace_back(330, 180);
        this->points.emplace_back(WIDTH / 2, 180);
        this->points.emplace_back(WIDTH / 2, 300);
        this->points.emplace_back(520, 300);
    } else {
        this->points.emplace_back(WIDTH / 2, 180);
        this->points.emplace_back(310, 180);
        this->points.emplace_back(120, 300);
        this->points.emplace_back(WIDTH / 2, 300);
    }

    this->mask = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1);

    this->set_mask();
}

/***
 * @brief Roi 영역을 이루는 네 꼭짓점 설정
 * @param points
 */
void Roi::set_points(std::vector<cv::Point> pts) {
    this->points.assign(pts.begin(), pts.end());
}

/***
 * @brief mask 행렬을 설정
 * @param pts Roi 영역을 나타내는 네 꼭짓점
 */
void Roi::set_mask() {
    fillPoly(this->mask, this->points, cv::Scalar(255), cv::LINE_AA);
    cv::bitwise_not(this->mask, this->mask);
}
