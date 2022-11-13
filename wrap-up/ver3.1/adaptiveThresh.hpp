#ifndef VER3_1_ADAPTIVETHRESH_HPP
#define VER3_1_ADAPTIVETHRESH_HPP

#include "opencv2/opencv.hpp"
#include "./constant.hpp"

/**
 * 동적 이진화 v1.0
 */
class AdaptiveThresh {
public:
    int thresh;

    AdaptiveThresh() {
        this->thresh = 150;
    }

    void applyThresholding(cv::InputArray src, cv::OutputArray dst);

    int updateThresholding(cv::InputArray src);
};

/**
 * 기존 thresholding 함수 대체 -> 호출 시점에 조정된 thresh 값으로 이진화
 * @param src
 * @param dst
 */
void AdaptiveThresh::applyThresholding(cv::InputArray src, cv::OutputArray dst) {
    threshold(src, dst, this->thresh, 145, cv::THRESH_BINARY);

}

/**
 * 이진화 값 조정하는 함수
 * @param src 이진화 된 프레임
 * @return 전체 픽셀 중 하얗게 표기 된 픽셀 수
 */
int AdaptiveThresh::updateThresholding(cv::InputArray src) {
    cv::Mat frame = src.getMat();
    int white = 0;
    for (int i = 0; i < DEFAULT_ROI_HEIGHT; i++) {
        uchar *ptr = frame.ptr<uchar>(i);
        for (int j = 0; j < DEFAULT_ROI_WIDTH; j++) {
            if (ptr[j]) white++;
        }
    }

#ifdef THRESH_DEBUG
    std::cout << this->thresh << ' ' << (double) white / (DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH) * 100 << ' ';
#endif
    // 5% 초과 혹은 1% 미만일 경우 임계치 조정
    if (white > DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH * 0.05) {
        this->thresh += 5;
    } else if (white < DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH * 0.01) {
        this->thresh -= 5;
    }

#ifdef THRESH_DEBUG
    std::cout << this->thresh << '\n';
#endif
    return white;
}

#endif //VER3_1_ADAPTIVETHRESH_HPP
