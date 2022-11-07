#ifndef PARK_ROI_HPP
#define PARK_ROI_HPP

#include <iostream>
#include <opencv2/opencv.hpp> // Mat, fillPoly, bitwise_and,
#include <vector>
#include "./latestInfo.cpp"
#include "./constant.hpp"

class ROI {
public:
    std::vector <cv::Point> default_ROI;
    bool adaptive_flag; // true: 동적 roi 적용중
    cv::Mat ROI_mask; // and 연산할 mask 이미지
    LatestInfo line_info[2]; // 왼쪽, 오른쪽

#ifdef DEBUG
    // ROI 관련 통계
    struct Statistic {
        int staticROI, adaptiveROI; // 동적, 정적 roi 적용 cnt
        int one_detected, zero_detected; // 차로 0개, 1개 탐지
    };

    Statistic stat;
#endif

    ROI();

    void initROI();

    void updateAdaptiveMask();

    void applyROI(cv::InputArray frame, cv::OutputArray result);

    void updateROI();

};

/**
 * 기본 생성자
 */
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

/**
 * roi 초기화 (정적 roi로 설정)
 */
void ROI::initROI() {
    this->ROI_mask = cv::Mat::zeros(FRAME_ROWS, FRAME_COLS, CV_8U);
    cv::fillPoly(this->ROI_mask, this->default_ROI, 255);
    adaptive_flag = false;
}

/**
 * roi mask를 프레임에 적용
 * @param frame 1채널 이미지
 * @param result 결과 받아갈 이미지
 */
void ROI::applyROI(cv::InputArray frame, cv::OutputArray result) {
    cv::bitwise_and(frame, this->ROI_mask, result);
}

/**
 * latest_info를 이용해서 동적 마스킹 영역 갱신
 */
void ROI::updateAdaptiveMask() {
    // 왼쪽 차선, 오른쪽 차선을 위한 마스크 생성
    cv::Mat mask[2];

    for (int i = 0; i < 2; i++) {
        mask[i] = cv::Mat::zeros(FRAME_ROWS, FRAME_COLS, CV_8U);
        Avg avg = line_info[i].get_avg();
        std::vector <cv::Point> polygon;

        int x1 = avg.coordX; // roi 밑변과의 교차점
        int x2 = avg.coordX - (DEFAULT_ROI_HEIGHT) * avg.gradient; // roi 윗변과의 교차점
        polygon.assign({
                               {x1 - DX, DEFAULT_ROI_DOWN},
                               {x1 + DX, DEFAULT_ROI_DOWN},
                               {x2 + DX, DEFAULT_ROI_UP},
                               {x2 - DX, DEFAULT_ROI_UP}
                       });
        cv::fillPoly(mask[i], polygon, 255);
    }
    // 왼쪽, 오른쪽 마스크 합치기
    bitwise_or(mask[0], mask[1], this->ROI_mask);
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

    // 정적 roi 리셋이 필요한 경우
    if (this->line_info[0].undetected_cnt == UNDETECTED_STD ||
        this->line_info[1].undetected_cnt == UNDETECTED_STD) {
        for (int i = 0; i < 2; i++) {
            this->line_info[i].reset();
        }
        this->initROI();
    }
    // 동적 roi 갱신
    if (this->line_info[0].adaptive_ROI_flag && this->line_info[1].adaptive_ROI_flag) {
        this->adaptive_flag = true;
        this->updateAdaptiveMask();
    }

}

#endif //PARK_ROI_HPP
