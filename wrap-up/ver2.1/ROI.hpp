#ifndef VER2_1_ROI_HPP
#define VER2_1_ROI_HPP

#include <iostream>
#include <opencv2/opencv.hpp> // Mat, fillPoly, bitwise_and,
#include <vector>
#include "./latestInfo.hpp"
#include "./constant.hpp"

class ROI {
public:
    // 좌우 하나씩
    std::vector <cv::Point> default_ROI[2]; // roi 영역
    bool adaptive_flag[2]; // true: 동적 roi 적용중
    cv::Mat ROI_mask[2];
    LatestInfo line_info[2];

#ifdef DEBUG
    // ROI 관련 통계
    struct Statistic {
        int staticROI, adaptiveROI; // 동적, 정적 roi 적용 cnt
        int one_detected, zero_detected; // 차로 0개, 1개 탐지
    };

    Statistic stat;
#endif

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
        default_ROI[i].resize(4);
        line_info[i].reset();
        adaptive_flag[i] = false;
    }
    default_ROI[0] = {
            {275,      DEFAULT_ROI_UP},
            {275 + 50, DEFAULT_ROI_UP},
            {187 + 50, DEFAULT_ROI_DOWN},
            {187,      DEFAULT_ROI_DOWN}
    };
    default_ROI[1] = {
            {342 - 50, DEFAULT_ROI_UP},
            {342,      DEFAULT_ROI_UP},
            {455,      DEFAULT_ROI_DOWN},
            {455 - 50, DEFAULT_ROI_DOWN}
    };

    for (int i = 0; i < 2; i++) {
        initROI(i);
    }

#ifdef DEBUG
    this->stat = {0, 0, 0, 0};
#endif
}

/**
 * roi 초기화 (정적 roi로 설정)
 */
void ROI::initROI(int pos) {
    this->ROI_mask[pos] = cv::Mat::zeros(FRAME_ROWS, FRAME_COLS, CV_8U);
    cv::fillPoly(this->ROI_mask[pos], this->default_ROI[pos], 255);
    adaptive_flag[pos] = false;
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
    this->ROI_mask[pos] = cv::Mat::zeros(FRAME_ROWS, FRAME_COLS, CV_8U);
    Avg avg = line_info[pos].get_avg();
    std::vector <cv::Point> polygon;

    int x1 = avg.coordX; // roi 밑변과의 교차점
    int x2 = x1 - (DEFAULT_ROI_HEIGHT) * avg.gradient; // roi 윗변과의 교차점
    polygon.assign({
                           {x1 - DX, DEFAULT_ROI_DOWN},
                           {x1 + DX, DEFAULT_ROI_DOWN},
                           {x2 + DX, DEFAULT_ROI_UP},
                           {x2 - DX, DEFAULT_ROI_UP}
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
#ifdef DEBUG
    if (this->line_info[0].adaptive_ROI_flag && this->line_info[1].adaptive_ROI_flag) this->stat.adaptiveROI++;
    else {
        this->stat.staticROI++;
    }
#endif
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

#endif //VER2_1_ROI_HPP
