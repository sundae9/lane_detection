#include <iostream>

#include <opencv2/opencv.hpp>

const int PROC_SIZE = 20;

/**
 * 각 프로세스 별 시간의 최소, 최대, 총합
 */
struct
{
    double min_time[PROC_SIZE];   // 각 프로세스 별 최소 시간
    double max_time[PROC_SIZE];   // 각 프로세스 별 최대 시간
    double total_time[PROC_SIZE]; // 각 프로세스 합계 (평균 시간 측정용)
} typedef Time_info;

/**
 * 각 프로세스 별 최소, 최대일 때 프레임 인덱스 정보
 */
struct
{
    int min_frame[PROC_SIZE]; // 각 프로세스 별 시간이 최소일 때 프레임 인덱스
    int max_frame[PROC_SIZE]; // 각 프로세스 별 시간이 최대일 때 프레임 인덱스
} typedef Frame_info;

/**
 * 영상 처리 정보 - 프로세스 별 시간 정보 및 프레임 정보 포함
 */
struct
{
    Time_info time_info;   // 시간 정보
    Frame_info frame_info; // 프레임 정보
    int total_frame;       // 전체 프레임 수
    int test_case;
    cv::Mat prev_img;
} typedef Video_info;

/**
 * @brief 초기화하는 함수 (min: 1000, max: -1)
 */
void Initialize_Video_info(Video_info &vi, int test_case)
{
    for (int i = 0; i < PROC_SIZE; i++)
    {
        vi.time_info.min_time[i] = 1000;
        vi.time_info.max_time[i] = -1;
        vi.time_info.total_time[i] = 0;
    }
    vi.total_frame = 0;
    vi.test_case = test_case;
}

/**
 * 타이머 정지 후 최대, 최소, 총합 갱신하고 타이머 재시작
 * @param tm 타이머
 * @param vi 영상 처리 정보 객체
 * @param idx 프로세스 인덱스
 */
void Time_record(cv::TickMeter &tm, Video_info &vi, int idx, cv::Mat frame)
{
    tm.stop();
    double cur_time = tm.getTimeMilli();
    // 첫번째 프레임은 처리 시간이 길기 때문에 통계에서 제외함
    if (vi.total_frame == 1)
    {
        tm.reset();
        tm.start();
        return;
    }

    // 최대값 갱신
    if (vi.time_info.max_time[idx] < cur_time)
    {
        vi.time_info.max_time[idx] = cur_time;
        vi.frame_info.max_frame[idx] = vi.total_frame;
        cv::imwrite("../result/tmp/" + std::to_string(vi.test_case) + "/" + std::to_string(idx) + "_max_prev.jpg", vi.prev_img);
        cv::imwrite("../result/tmp/" + std::to_string(vi.test_case) + "/" + std::to_string(idx) + "_max.jpg", frame);
    }

    // 최솟값 갱신
    if (vi.time_info.min_time[idx] > cur_time)
    {
        vi.time_info.min_time[idx] = cur_time;
        vi.frame_info.min_frame[idx] = vi.total_frame;
        cv::imwrite("../result/tmp/" + std::to_string(vi.test_case) + "/" + std::to_string(idx) + "_min_prev.jpg", vi.prev_img);
        cv::imwrite("../result/tmp/" + std::to_string(vi.test_case) + "/" + std::to_string(idx) + "_min.jpg", frame);
    }

    // 총합 갱신
    vi.time_info.total_time[idx] += cur_time;
    // 타이머 재시작
    vi.prev_img = frame.clone();
    tm.reset();
    tm.start();
}

/**
 * @brief 기록한 정보 출력하는 함수 -> 디버깅 용
 * @param vi 영상 처리 정보
 * @param idx 출력할 프로세스 "인덱스"
 */
void Print_info(Video_info &vi, int idx)
{
    std::cout << idx << ",";                                                               // 프로세스 번호
    std::cout << vi.time_info.min_time[idx] << "," << vi.frame_info.min_frame[idx] << ","; // 최소 정보
    std::cout << vi.time_info.max_time[idx] << "," << vi.frame_info.max_frame[idx] << ","; // 최대 정보
    std::cout << vi.time_info.total_time[idx] / (vi.total_frame - 1) << ",\n";             // 평균 정보
}

/**
 * @brief 기록한 정보 출력하는 함수 -> csv 전환용
 * @param vi 영상 처리 정보
 * @param cnt 전체 프로세스 수!! (인덱스 X)
 */
void Print_info_all(Video_info &vi, int cnt)
{
    for (int i = 0; i < cnt; i++)
    {
        std::cout << i << ",";                                                             // 프로세스 번호
        std::cout << vi.time_info.min_time[i] << "," << vi.frame_info.min_frame[i] << ","; // 최소 정보
        std::cout << vi.time_info.max_time[i] << "," << vi.frame_info.max_frame[i] << ","; // 최대 정보
        std::cout << vi.time_info.total_time[i] / (vi.total_frame - 1) << ",\n";           // 평균 정보
    }
}
