#ifndef VER3_1_LATESTINFO_HPP
#define VER3_1_LATESTINFO_HPP

#include <iostream>
#include "./constant.hpp"

#define QUE_MAX_SIZE 5
#define UNDETECTED_STD 10

struct Line_info {
    int x_bottom, x_top;
    double gradient;
};

class LatestInfo {
public:
    int undetected_cnt;             // 연속으로 선분을 찾지 못한 프레임의 수
    int que_length;                 // 큐에 들어있는 프레임의 수
    bool adaptive_ROI_flag;         // 동적 roi 플래그 표시
    Line_info line;

    void update_lines(cv::Point p1, cv::Point p2, double gradient);  // 차선 정보 갱신
//    Avg get_prev();                    // x 좌표와 기울기의 평균 반환
    void print_all();                 // 정보 출력 - 디버깅용
    void not_found();                 // 차선 미발견
    void reset();                     // 초기 실행시, 혹은 반복문 초입에서
};

void LatestInfo::update_lines(cv::Point p1, cv::Point p2, double gradient) {
    // 차선을 찾은 경우 - reset undetected_cnt
    this->undetected_cnt = 0;

    int x1 = (p1.x * (p2.y - DEFAULT_ROI_HEIGHT) - p2.x * (p1.y - DEFAULT_ROI_HEIGHT)) / (p2.y - p1.y);
    int x2 = x1 - DEFAULT_ROI_HEIGHT * gradient;

    if (this->que_length == 0) {
        this->line = {x1, x2, gradient};
        this->que_length++;
        return;
    }
    // 정보 갱신
    this->line.x_bottom = (4 * this->line.x_bottom + x1) / 5;
    this->line.x_top = (4 * this->line.x_top + x2) / 5;
    this->line.gradient = (4 * this->line.gradient + gradient) / 5;

    // 적응형 roi on
    if (!this->adaptive_ROI_flag && ++(this->que_length) == QUE_MAX_SIZE) {
        this->adaptive_ROI_flag = true;
    }
}

// 변경 -> 리셋 부분은 ROI에서 관리 - 좌우 차선의 리셋 지점을 맞춰주기 위함
void LatestInfo::not_found() {
    this->undetected_cnt++;
}

/**
 * 프린트
 */
void LatestInfo::print_all() {
    printf("undetected_cnt: %d\n", this->undetected_cnt);
    printf("que_length: %d\n", this->que_length);
    printf("undetected_cnt: %d\n", this->adaptive_ROI_flag);

    printf("|%10s|%10s|%10s\n", "x_bottom", "x_top", "grad");
    printf("|%10d|%10d|%10f\n", this->line.x_bottom, this->line.x_top, this->line.gradient);
}

/**
 * 초기화 하는 함수
 */
void LatestInfo::reset() {
    this->undetected_cnt = 0;
    this->que_length = 0;
    this->adaptive_ROI_flag = false;
    this->line = {0, 0, 0};
}

#endif //VER3_1_LATESTINFO_HPP
