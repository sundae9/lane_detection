#include <iostream>

#include "opencv2/core/utility.hpp"

const int proc_size = 20;

/**
 * 각 프로세스 별 시간의 최소, 최대, 총합
 */
struct
{
    double min_time[proc_size];    // 각 프로세스 별 최소 시간
    double max_time[proc_size];    // 각 프로세스 별 최대 시간
    double total_time[proc_size];  // 각 프로세스 합계 (평균 시간 측정용)
} typedef Time_info;

/**
 * 각 프로세스 별 최소, 최대일 때 프레임 인덱스 정보
 */
struct
{
    int min_frame[proc_size];   // 각 프로세스 별 시간이 최소일 때 프레임 인덱스
    int max_frame[proc_size];   // 각 프로세스 별 시간이 최대일 때 프레임 인덱스
} typedef Frame_index_info;

/**
 * 영상 처리 정보 - 프로세스 별 시간 정보 및 프레임 정보 포함
 */
struct
{
    Time_info time_info;    // 시간 정보
    Frame_info frame_info;  // 프레임 정보
    int total_frame;    // 전체 프레임 수
} typedef Video_info;

/**
 * 타이머 정지 후 최대, 최소, 총합 갱신
 * @param tm 타이머
 * @param video_info 영상 처리 정보 객체
 * @param idx 프로세스 인덱스
 */
void Time_record(cv::TickMeter &tm, Video_info &video_info, int idx) {
    tm.stop();
    double cur_time = tm.getTimeMilli();

    // 최대값 갱신
    if (video_info.total_frame != 1 && video_info.time_info.max_time[idx] < cur_time) {
        video_info.time_info.max_time[idx] = cur_time;
        video_info.frame_info.max_frame[idx] = video_info.frame_info.total_frame;
    }
    // 최솟값 갱신
    if (video_info.time_info.min_time[idx] > cur_time) {
        video_info.time_info.min_time[idx] = cur_time;
        video_info.frame_info.min_frame[idx] = video_info.frame_info.total_frame;
    }

    // 총합 갱신
    video_info.time_info.total_time[idx] += tm.getTimeMilli();
    tm.reset();
    tm.start();
}

void Print_info(Video_info &video_info, int idx) {
    std::cout << idx << ",";
    std::cout << video_info.time_info.min_time[idx] << "," << video_info.frame_info.min_frame[idx] << ",";
    std::cout << video_info.time_info.max_time[idx] << "," << video_info.frame_info.max_frame[idx] << ",";
    std::cout << video_info.time_info.total_time[idx] / video_info.frame_info.total_frame << ",\n";
}
