#ifndef PARK_ROI_HPP
#define PARK_ROI_HPP

#include <iostream>
#include <opencv2/opencv.hpp> // Mat, fillPoly, bitwiseand,
#include <vector>
#include "../latestInfo.cpp"
#include "./constant.hpp"

class ROI {
public:
    std::vector<cv::Point> default_ROI;
    bool adaptive_flag;
    cv::Mat ROI_mask;
    LatestInfo line_info[2];

#ifdef DEBUG
    struct Statistic {
        int staticROI, adaptiveROI;
        int one_detected, zero_detected;
    };
    Statistic stat;
#endif

    ROI();

    void initROI();

    void updateAdaptiveMask();

    void applyROI(cv::InputArray frame, cv::OutputArray result);

    void updateROI();

};

ROI::ROI() {
    default_ROI.resize(4);
    default_ROI = {
            {275, DEFAULT_ROI_UP},
            {342, DEFAULT_ROI_UP},
            {461, DEFAULT_ROI_DOWN},
            {187, DEFAULT_ROI_DOWN}
    };
    for (int i = 0; i < 2; i++) {
        line_info[i].reset();
    }
#ifdef DEBUG
    this->stat = {0, 0, 0, 0};
#endif
}

void ROI::initROI() {
    this->ROI_mask = cv::Mat::zeros(FRAME_ROWS, FRAME_COLS, CV_8U);
    cv::fillPoly(this->ROI_mask, this->default_ROI, 255);
    adaptive_flag = false;
}

void ROI::applyROI(cv::InputArray frame, cv::OutputArray result) {
    cv::bitwise_and(frame, this->ROI_mask, result);
}

void ROI::updateAdaptiveMask() {
    // 왼쪽 차선, 오른쪽 차선을 위한 마스크 생성
    cv::Mat mask[2];
    for (int i = 0; i < 2; i++) {
        mask[i] = cv::Mat::zeros(FRAME_ROWS, FRAME_COLS, CV_8U);
        Avg avg = line_info[i].get_avg();
        std::vector<cv::Point> polygon;
//        line_info[i].print_all();
//        std::cout << "side " << i << ' ';
        int x1 = avg.coordX;
        int x2 = avg.coordX - (DEFAULT_ROI_DOWN - DEFAULT_ROI_UP) * avg.gradient;
        polygon.assign({
                               {x1 - DX, DEFAULT_ROI_DOWN},
                               {x1 + DX, DEFAULT_ROI_DOWN},
                               {x2 + DX, DEFAULT_ROI_UP},
                               {x2 - DX, DEFAULT_ROI_UP}
                       });
        cv::fillPoly(mask[i], polygon, 255);
    }
    bitwise_or(mask[0], mask[1], this->ROI_mask);
}

void ROI::updateROI() {
    // case 1. 동적 - 동적
    // case 3. 정적 - 동적
    // 위 두 경우는 모두 roi update 필요
#ifdef DEBUG
    if (this->line_info[0].adaptive_ROI_flag && this->line_info[1].adaptive_ROI_flag) this->stat.adaptiveROI++;
    else {
        this->stat.staticROI++;
    }
#endif

    // 동적 roi 갱신
    if (this->line_info[0].adaptive_ROI_flag && this->line_info[1].adaptive_ROI_flag) {
        this->adaptive_flag = true;
        this->updateAdaptiveMask();
    }
        // 정적 roi 리셋이 필요한 경우 -> 둘 중 하나라도 0이면 init (둘이 동시에 카운트를 올려줘야 여기 걸리지 않음)
    else if (this->line_info[0].undetected_cnt == UNDETECTED_STD ||
             this->line_info[1].undetected_cnt == UNDETECTED_STD) {
        for (int i = 0; i < 2; i++) {
            this->line_info[i].reset();
        }
        this->initROI();
    }
}

#endif //PARK_ROI_HPP
