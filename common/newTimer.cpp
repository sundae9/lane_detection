#include <iostream>
#include <opencv2/opencv.hpp>

const int PROC_MAX_SIZE = 20;

/**
 * 각 프로세스 별 시간의 최소, 최대, 총합
 */
struct Time_info {
    double min_time[PROC_MAX_SIZE];   // 각 프로세스 별 최소 시간
    double max_time[PROC_MAX_SIZE];   // 각 프로세스 별 최대 시간
    double total_time[PROC_MAX_SIZE]; // 각 프로세스 합계 (평균 시간 측정용)
};

/**
 * 각 프로세스 별 최소, 최대일 때 프레임 인덱스 정보
 */
struct Frame_info {
    int min_frame[PROC_MAX_SIZE]; // 각 프로세스 별 시간이 최소일 때 프레임 인덱스
    int max_frame[PROC_MAX_SIZE]; // 각 프로세스 별 시간이 최대일 때 프레임 인덱스
};

/**
 * 시간 측정 클래스
 */
class TimeLapse {
public:
    cv::TickMeter tm_proc;  // 프로세스 별 타이머
    cv::TickMeter tm_total; // 전체 시간 타이머

    Time_info time_info;   // 시간 정보
    Frame_info frame_info; // 프레임 정보
    int total_frame;       // 전체 프레임 수
    int test_case;         // 테스트 케이스 번호
    int proc_cnt;          // 전체 프로세스 수
    int proc_idx;
    cv::Mat prev_img;      // 직전 처리된 이미지 저장

    /**
     * 생성자
     * @param proc_cnt 전체 프로세스 수
     */
    TimeLapse(int proc_cnt) {
        for (int i = 0; i < PROC_MAX_SIZE; i++) {
            this->time_info.min_time[i] = 1000;
            this->time_info.max_time[i] = -1;
            this->time_info.total_time[i] = 0;
        }
        this->total_frame = 0;
        this->proc_cnt = proc_cnt;
        this->proc_idx = 0;
    }

    /**
     * 테스트케이스 번호 설정 - 결과 저장용
     * @param tc
     */
    void set_tc(int tc) {
        this->test_case = tc;
    }

    void time_record(double cur_time, int idx, cv::Mat frame);

    void proc_record(cv::Mat frame);

    void total_record(cv::Mat original, cv::Mat result);

    void print_info(int idx);

    void print_info_all();

    void stop_both_timer();

    void restart();
};

/**
 * 타이머 측정값으로 최대, 최소, 총합 갱신 및 이미지 저장
 * @param cur_time 타이머 측정 값
 * @param proc_idx 프로세스 인덱스
 * @param frame
 */
void TimeLapse::time_record(double cur_time, int proc_idx, cv::Mat frame) {
    // 첫번째 프레임은 처리 시간이 길기 때문에 통계에서 제외함
    if (this->total_frame == 1) {
        return;
    }

    // 프레임 저장 경로
    std::string file_path =
            "../result/tmp/proc" + std::to_string(proc_idx) + "/tc" + std::to_string(this->test_case) + "_frame" +
            std::to_string(this->total_frame);

    // 최대값 갱신
    if (this->time_info.max_time[proc_idx] < cur_time) {
        this->time_info.max_time[proc_idx] = cur_time;
        this->frame_info.max_frame[proc_idx] = this->total_frame;
        cv::imwrite(file_path + "_max_prev.jpg", this->prev_img);
        cv::imwrite(file_path + "_max.jpg", frame);
    }

    // 최솟값 갱신
    if (this->time_info.min_time[proc_idx] > cur_time) {
        this->time_info.min_time[proc_idx] = cur_time;
        this->frame_info.min_frame[proc_idx] = this->total_frame;
        cv::imwrite(file_path + "_min_prev.jpg", this->prev_img);
        cv::imwrite(file_path + "_min.jpg", frame);
    }

    // 총합 갱신
    this->time_info.total_time[proc_idx] += cur_time;
    this->prev_img = frame.clone();
}

/**
 * 프로세스 기록 측정
 * @param proc_idx
 * @param frame
 */
void TimeLapse::proc_record(cv::Mat frame) {
    this->stop_both_timer();
    double cur_time = this->tm_proc.getTimeMilli();
    this->time_record(cur_time, this->proc_idx, frame);
    this->tm_proc.reset();
    this->tm_proc.start();
    this->tm_total.start();
    this->proc_idx++;
}

/**
 * 전체 기록 측정 (한 프레임을 처리하는데 걸린 총 시간)
 * @param original 처리 전 원본 프레임
 * @param result 처리 후 결과 프레임
 */
void TimeLapse::total_record(cv::Mat original, cv::Mat result) {
    this->stop_both_timer();
    double cur_time = this->tm_total.getTimeMilli();
    // 처리 전 이미지를 원본으로 바꿔둠
    this->prev_img = original.clone();
    this->time_record(cur_time, this->proc_cnt, result);
}

/**
 * @brief 기록한 정보 출력하는 함수 -> 디버깅 용
 * @param proc_idx 출력할 프로세스 "인덱스"
 */
void TimeLapse::print_info(int idx) {
    std::cout << idx << ",";                                                               // 프로세스 번호
    std::cout << this->time_info.min_time[idx] << "," << this->frame_info.min_frame[idx] << ","; // 최소 정보
    std::cout << this->time_info.max_time[idx] << "," << this->frame_info.max_frame[idx] << ","; // 최대 정보
    std::cout << this->time_info.total_time[idx] / (this->total_frame - 1) << ",\n";// 평균 정보 -1번 프레임은 통계에서 제외
}

/**
 * @brief 기록한 정보 출력하는 함수 -> csv 전환용
 */
void TimeLapse::print_info_all() {
    for (int i = 0; i <= this->proc_cnt; i++) {
        print_info(i);
    }
}

void TimeLapse::stop_both_timer() {
    this->tm_proc.stop();
    this->tm_total.stop();
}

void TimeLapse::restart() {
    this->tm_proc.reset();
    this->tm_total.reset();
    this->tm_proc.start();
    this->tm_total.start();

    this->total_frame++;
    this->proc_idx = 0;
}