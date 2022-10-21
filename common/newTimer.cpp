#include <iostream>

#include <opencv2/opencv.hpp>

const int PROC_SIZE = 20;

/**
 * 각 프로세스 별 시간의 최소, 최대, 총합
 */
struct {
    double min_time[PROC_SIZE];   // 각 프로세스 별 최소 시간
    double max_time[PROC_SIZE];   // 각 프로세스 별 최대 시간
    double total_time[PROC_SIZE]; // 각 프로세스 합계 (평균 시간 측정용)
} typedef Time_info;

/**
 * 각 프로세스 별 최소, 최대일 때 프레임 인덱스 정보
 */
struct {
    int min_frame[PROC_SIZE]; // 각 프로세스 별 시간이 최소일 때 프레임 인덱스
    int max_frame[PROC_SIZE]; // 각 프로세스 별 시간이 최대일 때 프레임 인덱스
} typedef Frame_info;

/**
 * 영상 처리 정보 - 프로세스 별 시간 정보 및 프레임 정보 포함
 */
class Video_info {
public:
    cv::TickMeter tm_proc;
    cv::TickMeter tm_total;

    Time_info time_info;   // 시간 정보
    Frame_info frame_info; // 프레임 정보
    int total_frame;       // 전체 프레임 수
    int test_case;
    int proc_cnt;
    cv::Mat prev_img;

    Video_info(int proc_cnt) {
        for (int i = 0; i < PROC_SIZE; i++) {
            this->time_info.min_time[i] = 1000;
            this->time_info.max_time[i] = -1;
            this->time_info.total_time[i] = 0;
        }
        this->total_frame = 0;
        this->proc_cnt = proc_cnt;
    }

    void set_tc(int tc) {
        this->test_case = tc;
    }

    void time_record(cv::TickMeter &tm, int idx, cv::Mat frame);

    void proc_record(int idx, cv::Mat frame);

    void total_record(cv::Mat frame);

    void print_info(int idx);

    void print_info_all();

    void stop_timer();

    void start_timer();
};

/**
 * 타이머 정지 후 최대, 최소, 총합 갱신하고 타이머 재시작
 * @param tm 타이머
 * @param vi 영상 처리 정보 객체
 * @param idx 프로세스 인덱스
 */
void Video_info::time_record(cv::TickMeter &tm, int idx, cv::Mat frame) {
    tm.stop();
    double cur_time = tm.getTimeMilli();
    // 첫번째 프레임은 처리 시간이 길기 때문에 통계에서 제외함
    if (this->total_frame == 1) {
        tm.reset();
        tm.start();
        return;
    }

    // 최대값 갱신
    if (this->time_info.max_time[idx] < cur_time) {
        this->time_info.max_time[idx] = cur_time;
        this->frame_info.max_frame[idx] = this->total_frame;
        cv::imwrite("../result/tmp/" + std::to_string(this->test_case) + "/" + std::to_string(idx) + "_max_prev.jpg",
                    this->prev_img);
        cv::imwrite("../result/tmp/" + std::to_string(this->test_case) + "/" + std::to_string(idx) + "_max.jpg", frame);
    }

    // 최솟값 갱신
    if (this->time_info.min_time[idx] > cur_time) {
        this->time_info.min_time[idx] = cur_time;
        this->frame_info.min_frame[idx] = this->total_frame;
        cv::imwrite("../result/tmp/" + std::to_string(this->test_case) + "/" + std::to_string(idx) + "_min_prev.jpg",
                    this->prev_img);
        cv::imwrite("../result/tmp/" + std::to_string(this->test_case) + "/" + std::to_string(idx) + "_min.jpg", frame);
    }

    // 총합 갱신
    this->time_info.total_time[idx] += cur_time;
    // 타이머 재시작
    this->prev_img = frame.clone();
    tm.reset();
    tm.start();
}

/**
 * 타이머 정지 후 최대, 최소, 총합 갱신하고 타이머 재시작
 * @param tm 타이머
 * @param vi 영상 처리 정보 객체
 * @param idx 프로세스 인덱스
 */
void Video_info::proc_record(int idx, cv::Mat frame) {
    time_record(this->tm_proc, idx, frame);
}

void Video_info::total_record(cv::Mat frame) {
    time_record(this->tm_total, this->proc_cnt, frame);
}

/**
 * @brief 기록한 정보 출력하는 함수 -> 디버깅 용
 * @param vi 영상 처리 정보
 * @param idx 출력할 프로세스 "인덱스"
 */
void Video_info::print_info(int idx) {
    std::cout << idx << ",";                                                               // 프로세스 번호
    std::cout << this->time_info.min_time[idx] << "," << this->frame_info.min_frame[idx] << ","; // 최소 정보
    std::cout << this->time_info.max_time[idx] << "," << this->frame_info.max_frame[idx] << ","; // 최대 정보
    std::cout << this->time_info.total_time[idx] / (this->total_frame - 1) << ",\n";             // 평균 정보
}

/**
 * @brief 기록한 정보 출력하는 함수 -> csv 전환용
 * @param vi 영상 처리 정보
 * @param cnt 전체 프로세스 수!! (인덱스 X)
 */
void Video_info::print_info_all() {
    for (int i = 0; i <= this->proc_cnt; i++) {
        print_info(i);
    }
}

void Video_info::stop_timer() {
    this->tm_total.stop();
    this->tm_proc.stop();
}

void Video_info::start_timer() {
    this->tm_total.reset();
    this->tm_proc.reset();
    this->tm_total.start();
    this->tm_proc.start();
}