#include <iostream>

const int QUE_MAX_SIZE = 5;
const int UNDETECTED_STD = 3;

struct Line_info {
    int coordX;
    double gradient;
};

struct Sum {
    int coordXL; // 좌측 X좌표의 합계
    int coordXR; // 우측 X좌표의 합계
    double gradientL; // 좌측 기울기의 합계
    double gradientR; // 우측 기울기의 합계
};

struct Avg {
    double coordXL; // 좌측 x좌표
    double coordXR; // 우측 x좌표
    double gradientL; // 좌측 기울기
    double gradientR; // 우측 기울기
};

class LatestInfo {
public:
    Line_info left[QUE_MAX_SIZE]; // 좌측 선분정보 저장
    Line_info right[QUE_MAX_SIZE]; // 우측 선분정보 저장
    int update_idx; // 업데이트 해야 하는 큐의 인덱스
    int undetected_cnt; // 연속으로 선분을 찾지 못한 프레임의 수
    int que_length;     // 큐에 들어있는 프레임의 수
    Sum sum; // 합계 정보
    bool adaptive_ROI_flag;  // 동적 roi 플래그 표시

    void update_lines(Line_info li_left, Line_info li_right); // 차선 정보 갱신
    Avg get_avg(); // 좌측, 우측의 X좌표와 기울기의 평균 반환
    void print_all(); // 정보 출력 - 디버깅용
    void not_found(); // 차선 미발견
    void reset(); // 초기 실행시, 혹은

    LatestInfo() {
        this->reset();
    }
};

void LatestInfo::update_lines(Line_info li_left, Line_info li_right) {
    // 차선을 찾은 경우 - reset undetected_cnt
    this->undetected_cnt = 0;

    // 합계 정보 갱신
    this->sum.coordXL += li_left.coordX - this->left[this->update_idx].coordX;
    this->sum.gradientL += li_left.gradient - this->left[this->update_idx].gradient;
    this->sum.coordXR += li_right.coordX - this->right[this->update_idx].coordX;
    this->sum.gradientR += li_right.gradient - this->right[this->update_idx].gradient;

    // 큐에 삽입
    this->left[this->update_idx] = li_left;
    this->right[this->update_idx] = li_right;

    // 인덱스 갱신
    this->update_idx = (this->update_idx + 1) % QUE_MAX_SIZE;

    // 적응형 roi on
    if (!this->adaptive_ROI_flag && ++this->que_length == QUE_MAX_SIZE) {
        this->adaptive_ROI_flag = true;
    }
}


void LatestInfo::not_found() {
    this->undetected_cnt++;
    if (this->undetected_cnt == UNDETECTED_STD) {
        this->reset();
    }
}

/**
 * 프린트
 */
void LatestInfo::print_all() {
    int tmp_idx;
    printf("que_length: %d\n", this->que_length);
    printf("frame num|%10s|%10s|%10s|%10s\n", "left-x", "grad", "right-x", "grad");
    for (int i = 0; i < QUE_MAX_SIZE; i++) {
        tmp_idx = (update_idx - i) % QUE_MAX_SIZE;
        printf("frame %2d |%10d|%10f|%10d|%10f\n", -i, this->left[tmp_idx].coordX, this->left[tmp_idx].gradient,
               this->right[tmp_idx].coordX, this->right[tmp_idx].gradient);
    }
    printf("total    |%10s|%10s|%10s|%10s\n", "left-x", "grad", "right-x", "grad");
    printf("frame    |%10d|%10f|%10d|%10f\n", this->sum.coordXL, this->sum.gradientL, this->sum.coordXR,
           this->sum.gradientR);
    Avg avg = this->get_avg();
    printf("Avg      |%10s|%10s|%10s|%10s\n", "left-x", "grad", "right-x", "grad");
    printf("frame    |%10f|%10f|%10f|%10f\n", avg.coordXL, avg.gradientL, avg.coordXR, avg.gradientR);
}

/**
 * @return 평균 정보
 */
Avg LatestInfo::get_avg() {
    return {
            (double) this->sum.coordXL / this->que_length,
            (double) this->sum.coordXR / this->que_length,
            this->sum.gradientL / this->que_length,
            this->sum.gradientR / this->que_length
    };
}

/**
 * 초기화하는 함수
 */
void LatestInfo::reset() {
    this->update_idx = 0;
    this->undetected_cnt = 0;
    this->que_length = 0;
    this->adaptive_ROI_flag = false;
    this->sum = {0, 0, 0, 0};
    memset(this->left, 0, sizeof(this->left));
    memset(this->right, 0, sizeof(this->right));
}