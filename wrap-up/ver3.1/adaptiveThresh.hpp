#ifndef VER3_1_ADAPTIVETHRESH_HPP
#define VER3_1_ADAPTIVETHRESH_HPP

#include "opencv2/opencv.hpp"
#include "./constant.hpp"

/**
 * 동적 이진화 v1.0
 */
class AdaptiveThresh {
public:
    int thresh[2]; // left, right

    AdaptiveThresh() {
        this->thresh[0] = 150;
        this->thresh[1] = 150;
    }

    void applyThresholding(cv::InputArray src, cv::OutputArray dst);

    std::pair<int, int> updateThresh(cv::InputArray src);
};

/**
 * 기존 thresholding 함수 대체 -> 호출 시점에 조정된 thresh 값으로 이진화
 * @param src
 * @param dst
 */
void AdaptiveThresh::applyThresholding(cv::InputArray src, cv::OutputArray dst) {
    int x[2] = {0, DEFAULT_ROI_WIDTH / 2};

    cv::Mat result[2];

    dst.getMat().resize(DEFAULT_ROI_WIDTH, DEFAULT_ROI_HEIGHT);

    for (int i = 0; i < 2; i++) {
        cv::Mat src_half = src.getMat()(cv::Rect(x[i], 0, DEFAULT_ROI_WIDTH / 2, DEFAULT_ROI_HEIGHT));
        cv::Mat dst_half = dst.getMat()(cv::Rect(x[i], 0, DEFAULT_ROI_WIDTH / 2, DEFAULT_ROI_HEIGHT));
        threshold(src_half, dst_half, this->thresh[i], 145, cv::THRESH_BINARY);
    }
}

/**
 * 이진화 값 조정하는 함수
 * @param src 이진화 된 프레임
 * @return 전체 픽셀 중 하얗게 표기 된 픽셀 수
 */
std::pair<int, int> AdaptiveThresh::updateThresh(cv::InputArray src) {
    cv::Mat frame = src.getMat();
    int white[2] = {0, 0};

    for (int i = 0; i < DEFAULT_ROI_HEIGHT; i++) {
        uchar *ptr = frame.ptr<uchar>(i);
        for (int j = 0; j < DEFAULT_ROI_WIDTH / 2; j++) {
            if (ptr[j]) white[0]++;
        }
        for (int j = DEFAULT_ROI_WIDTH / 2; j < DEFAULT_ROI_WIDTH; j++) {
            if (ptr[j]) white[1]++;
        }
    }

#ifdef THRESH_DEBUG
    std::cout << ' ' << (double) white[0] / (DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH) * 200 << ' '
              << (double) white[1] / (DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH) * 200 << ' ';
    std::cout << white[0] << ' ' << white[1] << ' ' << DEFAULT_ROI_WIDTH * DEFAULT_ROI_HEIGHT * 0.05 / 2 << ' ';
#endif

    for (int i = 0; i < 2; i++) {
        // 5% 초과 혹은 1% 미만일 경우 임계치 조정
        if (white[i] > DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH * 0.05 / 2) {
            this->thresh[i] += 5;
        } else if (white[i] < DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH * 0.01 / 2) {
            this->thresh[i] -= 5;
        }
    }

#ifdef THRESH_DEBUG
    std::cout << this->thresh[0] << ' ' << this->thresh[1] << '\n';
#endif
    return {white[0], white[1]};
}

#endif //VER3_1_ADAPTIVETHRESH_HPP
