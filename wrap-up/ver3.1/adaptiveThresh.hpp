#ifndef VER3_1_ADAPTIVETHRESH_HPP
#define VER3_1_ADAPTIVETHRESH_HPP

#include "opencv2/opencv.hpp"
#include "./constant.hpp"

/**
 * 동적 이진화 v1.0
 */
class AdaptiveThresh {
public:
    int thresh; // left, right
    int white; // pixel count

    AdaptiveThresh() {
        this->thresh = DEFAULT_BINARIZATION_THRESH;
        this->white = 0;
    }

    void applyThresholding(cv::InputArray src, cv::OutputArray dst, int pos);

    int updateThresh(cv::InputArray src, int pos, bool is_adaptive);

    int getThresh() {
        return this->thresh;
    }

    void setThresh(int newThresh) {
        this->thresh = newThresh;
    }
};

/**
 * 기존 thresholding 함수 대체 -> 호출 시점에 조정된 thresh 값으로 이진화
 * @param src
 * @param dst
 */
void AdaptiveThresh::applyThresholding(cv::InputArray src, cv::OutputArray dst, int pos) {
    int x[2] = {0, DEFAULT_ROI_WIDTH / 2};

    dst.getMat().resize(DEFAULT_ROI_WIDTH, DEFAULT_ROI_HEIGHT);

    cv::Mat src_half = src.getMat()(cv::Rect(x[pos], 0, DEFAULT_ROI_WIDTH / 2, DEFAULT_ROI_HEIGHT));
    cv::Mat dst_half = dst.getMat()(cv::Rect(x[pos], 0, DEFAULT_ROI_WIDTH / 2, DEFAULT_ROI_HEIGHT));
    threshold(src_half, dst_half, this->thresh, 145, cv::THRESH_BINARY);
}

/**
 * 이진화 값 조정하는 함수
 * @param src 이진화 된 프레임
 * @return 전체 픽셀 중 하얗게 표기 된 픽셀 수
 */
int AdaptiveThresh::updateThresh(cv::InputArray src, int pos, bool is_adaptive) {
    cv::Mat frame = src.getMat();
    int new_white = 0;
    int offset = pos == 0 ? 0 : DEFAULT_ROI_WIDTH / 2;

    for (int i = 0; i < DEFAULT_ROI_HEIGHT; i++) {
        uchar *ptr = frame.ptr<uchar>(i);
        for (int j = offset; j < offset + DEFAULT_ROI_WIDTH / 2; j++) {
            if (ptr[j]) new_white++;
        }
    }
    this->white = new_white;
#ifdef THRESH_DEBUG
    //    std::cout << ' ' << (double) white[0] / (DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH) * 200 << ' '
    //              << (double) white[1] / (DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH) * 200 << ' ';
    //    std::cout << white[0] << ' ' << white[1] << ' ' << DEFAULT_ROI_WIDTH * DEFAULT_ROI_HEIGHT * 0.05 / 2 << ' ';
#endif
    double upper, lower;

    upper = is_adaptive ? 0.05 : 0.1;
    lower = is_adaptive ? 0.01 : 0.015;

    // 5% 초과 혹은 1% 미만일 경우 임계치 조정
    if (this->white > DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH * upper / 2) {
        this->thresh += 5;
    } else if (this->white < DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH * lower / 2) {
        this->thresh -= 5;
    }

//#ifdef THRESH_DEBUG
//    std::cout << pos << ' ' << white << ' ' << this->thresh << '\n';
//#endif
    return this->white;
}

#endif //VER3_1_ADAPTIVETHRESH_HPP
