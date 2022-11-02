#include "Roi.h"

#include <opencv2/opencv.hpp>

/**
 * @brief 생성자. points, mask 기본 세팅
 * @param is_right 우측 roi 영역이라면 true
 */
Roi::Roi(bool is_right) {
    this->is_right = is_right;
    this->latest.reset();
    this->mask_reset();
}


void Roi::mask_reset() {
    this->points.clear();

    if (this->is_right) {
        this->points.emplace_back(WIDTH / 2 + HIGH_OFFSET, HIGH_POINT);
        this->points.emplace_back(WIDTH / 2, HIGH_POINT);
        this->points.emplace_back(WIDTH / 2, LOW_POINT);
        this->points.emplace_back(WIDTH / 2 + LOW_OFFSET, LOW_POINT);
    } else {
        this->points.emplace_back(WIDTH / 2, HIGH_POINT);
        this->points.emplace_back(WIDTH / 2 - HIGH_OFFSET, HIGH_POINT);
        this->points.emplace_back(WIDTH / 2 - LOW_OFFSET, LOW_POINT);
        this->points.emplace_back(WIDTH / 2, LOW_POINT);
    }

}

void Roi::calc_mask() {
    this->points.clear();

    Avg avg = this->latest.get_avg();

    int low_coordX = (int) (avg.coordX);
    int high_coordX = (int) ((LOW_POINT - HIGH_POINT + (avg.coordX * avg.gradient)) / avg.gradient);

    int offset = 25;

    if (this->is_right) {
        this->points.emplace_back(high_coordX + offset, HIGH_POINT);
        this->points.emplace_back(high_coordX - offset, HIGH_POINT);
        this->points.emplace_back(low_coordX - offset, LOW_POINT);
        this->points.emplace_back(low_coordX + offset, LOW_POINT);
    } else {
        this->points.emplace_back(high_coordX + offset, HIGH_POINT);
        this->points.emplace_back(high_coordX - offset, HIGH_POINT);
        this->points.emplace_back(low_coordX - offset, LOW_POINT);
        this->points.emplace_back(low_coordX + offset, LOW_POINT);
    }
}

/***
 * @brief LatestInfo의 adaptive_ROI_flag를 체크하고 mask 행렬을 설정
 * @param latest LatestInfo 구조체
 */
void Roi::set_mask() {
    if (this->latest.adaptive_ROI_flag) {
        calc_mask();
    } else {
        this->mask_reset();
    }

    this->mask = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC1);

    fillPoly(this->mask, this->points, cv::Scalar(255), cv::LINE_AA);
    cv::bitwise_not(this->mask, this->mask);
}
