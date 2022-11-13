#ifndef VER3_1_ROI_HPP
#define VER3_1_ROI_HPP

#include <iostream>
#include <opencv2/opencv.hpp> // Mat, fillPoly, bitwise_and,
#include <vector>
#include "./latestInfo.hpp"

class ROI {
public:
    // 좌우 하나씩
    cv::Mat default_mask[2];
    cv::Mat ROI_mask[2];
    LatestInfo line_info[2];

    ROI();

    void initROI(int pos);

    void updateAdaptiveMask(int pos);

    void applyROI(cv::InputArray frame, cv::OutputArray result);

    void updateROI();

};

/**
 * 기본 생성자
 */
ROI::ROI() {
    for (int i = 0; i < 2; i++) {
        this->default_mask[i] = cv::Mat::zeros(DEFAULT_ROI_HEIGHT, DEFAULT_ROI_WIDTH, CV_8U);
    }

    this->default_mask[0](cv::Range(0, DEFAULT_ROI_HEIGHT), cv::Range(0, DEFAULT_ROI_CENTER - DEFAULT_ROI_LEFT)).setTo(
            255);
    this->default_mask[1](cv::Range(0, DEFAULT_ROI_HEIGHT),
                          cv::Range(DEFAULT_ROI_CENTER - DEFAULT_ROI_LEFT, DEFAULT_ROI_WIDTH)).setTo(255);

    for (int i = 0; i < 2; i++) {
        line_info[i].reset();
        initROI(i);
    }
}

/**
 * roi 초기화 (정적 roi로 설정)
 */
void ROI::initROI(int pos) {
    this->ROI_mask[pos] = this->default_mask[pos].clone();
}

/**
 * roi mask를 프레임에 적용
 * @param frame 1채널 이미지
 * @param result 결과 받아갈 이미지
 */
void ROI::applyROI(cv::InputArray frame, cv::OutputArray result) {
    cv::Mat mask;
    cv::bitwise_or(this->ROI_mask[0], this->ROI_mask[1], mask);
    cv::bitwise_and(frame, mask, result);
}

/**
 * latest_info를 이용해서 동적 마스킹 영역 갱신
 */
void ROI::updateAdaptiveMask(int pos) {
    // 동적 roi mask 갱신
    this->ROI_mask[pos] = cv::Mat::zeros(DEFAULT_ROI_HEIGHT, DEFAULT_ROI_WIDTH, CV_8U);
    Line_info avg = line_info[pos].line;
    std::vector <cv::Point> polygon;

    polygon.assign({
                           {avg.x_bottom - DX, DEFAULT_ROI_HEIGHT},
                           {avg.x_bottom + DX, DEFAULT_ROI_HEIGHT},
                           {avg.x_top + DX,    0},
                           {avg.x_top - DX,    0}
                   });
    cv::fillPoly(this->ROI_mask[pos], polygon, 255);
}

/**
 * roi 갱신
 * - 정적 -> 동적: 동적 roi 갱신
 * - 동적 유지: 동적 roi 갱신 (직전에 업데이트가 일어나지 않았다면 continue 하는 조건 추가 예정)
 * - 동적 -> 정적: roi 초기화 및 line_info(latest_info) 초기화
 */
void ROI::updateROI() {
    for (int i = 0; i < 2; i++) {
        // 1. 정적 roi 리셋이 필요한 경우
        if (this->line_info[i].undetected_cnt == UNDETECTED_STD) {
            this->line_info[i].reset();
            this->initROI(i);
        } else if (this->line_info[i].undetected_cnt == 0 && this->line_info[i].adaptive_ROI_flag) {
            // 2. 동적 roi 갱신
            this->updateAdaptiveMask(i);
        }
    }
}

#endif //VER3_1_ROI_HPP
