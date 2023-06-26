#ifndef VER2_0_LATESTINFO_HPP
#define VER2_0_LATESTINFO_HPP

#include <iostream>

#define QUE_MAX_SIZE 5
#define UNDETECTED_STD 5

struct Line_info {
    int coordX;
    double gradient;
};

struct Sum {
    int coordX;       // X좌표의 합계
    double gradient;  // 기울기의 합계
};

struct Avg {
    int coordX;    // x좌표 평균 -> 이거 그냥 int형 써도 되지 않을까?
    double gradient;  // 기울기 평균
};

class LatestInfo {
public:
    Line_info lines[QUE_MAX_SIZE];  // 선분정보 저장
    int update_idx;                 // 업데이트 해야 하는 큐의 인덱스
    int undetected_cnt;             // 연속으로 선분을 찾지 못한 프레임의 수
    int que_length;                 // 큐에 들어있는 프레임의 수
    Sum sum;                        // 합계 정보
    bool adaptive_ROI_flag;         // 동적 roi 플래그 표시

    void update_lines(Line_info li);  // 차선 정보 갱신
    Avg get_avg();                    // x 좌표와 기울기의 평균 반환
    void print_all();                 // 정보 출력 - 디버깅용
    void not_found();                 // 차선 미발견
    void reset();                     // 초기 실행시, 혹은 반복문 초입에서
};

void LatestInfo::update_lines(Line_info li) {
    // 차선을 찾은 경우 - reset undetected_cnt
    this->undetected_cnt = 0;

    // 합계 정보 갱신
    this->sum.coordX += li.coordX - this->lines[this->update_idx].coordX;
    this->sum.gradient += li.gradient - this->lines[this->update_idx].gradient;

    // 큐에 삽입
    this->lines[this->update_idx] = li;

    // 인덱스 갱신
    this->update_idx = (this->update_idx + 1) % QUE_MAX_SIZE;

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
    int tmp_idx;
    printf("que_length: %d\n", this->que_length);
    printf("frame num|%10s|%10s\n", "x", "grad");
    for (int i = 0; i < QUE_MAX_SIZE; i++) {
        tmp_idx = (this->update_idx + i) % QUE_MAX_SIZE;
        printf("frame %2d |%10d|%10f\n", -i, this->lines[tmp_idx].coordX, this->lines[tmp_idx].gradient);
    }
    printf("total    |%10s|%10s\n", "left-x", "grad");
    printf("frame    |%10d|%10f\n", this->sum.coordX, this->sum.gradient);
    Avg avg = this->get_avg();
    printf("Avg      |%10s|%10s\n", "left-x", "grad");
    printf("frame    |%10d|%10f\n", avg.coordX, avg.gradient);
}

/**
 * @return 평균 정보
 */
Avg LatestInfo::get_avg() {
    return {
            this->sum.coordX / QUE_MAX_SIZE,
            this->sum.gradient / QUE_MAX_SIZE,
    };
}

/**
 * 초기화 하는 함수
 */
void LatestInfo::reset() {
    this->update_idx = 0;
    this->undetected_cnt = 0;
    this->que_length = 0;
    this->adaptive_ROI_flag = false;
    this->sum = {0, 0};
    memset(this->lines, 0, sizeof(this->lines));
}

#endif //VER2_0_LATESTINFO_HPP
