#ifndef VER2_2_LATESTINFO_HPP
#define VER2_2_LATESTINFO_HPP

#include <iostream>

#define QUE_MAX_SIZE 5
#define UNDETECTED_STD 5

struct Line_info {
    int coordX;
    double gradient;
};

class LatestInfo {
public:
    int undetected_cnt;             // 연속으로 선분을 찾지 못한 프레임의 수
    int que_length;                 // 큐에 들어있는 프레임의 수
    bool adaptive_ROI_flag;         // 동적 roi 플래그 표시
    Line_info line;

    void update_lines(Line_info li);  // 차선 정보 갱신
//    Avg get_prev();                    // x 좌표와 기울기의 평균 반환
    void print_all();                 // 정보 출력 - 디버깅용
    void not_found();                 // 차선 미발견
    void reset();                     // 초기 실행시, 혹은 반복문 초입에서
};

void LatestInfo::update_lines(Line_info li) {
    // 차선을 찾은 경우 - reset undetected_cnt
    this->undetected_cnt = 0;

    if (this->que_length == 0) {
        this->line = li;
        this->que_length++;
        return;
    }
    // 정보 갱신
    this->line.coordX = (4 * this->line.coordX + li.coordX) / 5;
    this->line.gradient = (4 * this->line.gradient + this->line.gradient) / 5;

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

    printf("|%10s|%10s\n", "x1", "grad");
    printf("|%10d|%10f\n", this->line.coordX, this->line.gradient);
}

/**
 * 초기화 하는 함수
 */
void LatestInfo::reset() {
    this->undetected_cnt = 0;
    this->que_length = 0;
    this->adaptive_ROI_flag = false;
    this->line = {0, 0};
}

#endif //VER2_2_LATESTINFO_HPP
