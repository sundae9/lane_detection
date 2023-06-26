#ifndef LANE_DETECTION_ONEVIDEOWRITER_HPP
#define LANE_DETECTION_ONEVIDEOWRITER_HPP

#include <iostream>
#include <opencv2/opencv.hpp>

/**
 * 영상 합본 저장하는 클래스
 */
class OneVideoWriter {
public:
    int width, height; // 영상 1개의 너비, 높이
    int row, col; // 영상 배치 레이아웃 - 2X3으로 배치한다면 row=2, col=3
    int proc_cnt; // 한 화면에 나올 영상 개수
    std::string file_path; // 비디오 저장 경로
    cv::Mat merged_frame; // 합본
    cv::VideoWriter vw;

    OneVideoWriter() {} // 선언용

    OneVideoWriter(std::string file_path, int width, int height, int row, int col, int proc_cnt);

    void writeFrame(cv::Mat frame, int idx);
};

/**
 * 생성자
 * @param file_path 저장 파일 경로
 * @param width 작은 영상 1개 너비
 * @param height 작은 영상 1개 높이
 * @param row 영상 레이아웃 - 몇개의 행으로 배치할건지
 * @param col 영상 레이아웃 - 몇개의 열로 배치할건지
 * @param proc_cnt 프로세수 수 - 영상의 수
 */
OneVideoWriter::OneVideoWriter(std::string file_path, int width, int height, int row, int col, int proc_cnt) {
    this->width = width; // 영상 1개의 너비
    this->height = height; // 영상 1개의 높이
    this->row = row;
    this->col = col;
    this->proc_cnt = proc_cnt;

    vw.open(file_path, cv::VideoWriter::fourcc('p', 'n', 'g', ' '), 30, cv::Size(col * width, row * height), true);
    this->merged_frame = cv::Mat(row * height, col * width, CV_8UC3);
}

/**
 * idx번째 프레임 저장하는 함수
 * @param frame
 * @param idx 0부터 시작하는 인덱스
 */
void OneVideoWriter::writeFrame(cv::Mat frame, int idx) {
    cv::Mat frame_cp = frame.clone();

    if (frame.type() != CV_8UC3) {
        cvtColor(frame, frame_cp, cv::COLOR_GRAY2BGR);
    }

    int r = idx / this->col, c = idx % this->col;

    cv::Mat roi = this->merged_frame(cv::Rect(c * this->width, r * this->height, this->width, this->height));
    frame_cp.copyTo(roi);

    if (idx + 1 == this->proc_cnt) {
        this->vw << this->merged_frame;
    }
}

#endif //LANE_DETECTION_ONEVIDEOWRITER_HPP
