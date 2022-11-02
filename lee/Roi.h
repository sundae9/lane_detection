#ifndef LEE_ROI_H
#define LEE_ROI_H

#define WIDTH 640
#define HEIGHT 360

#define HIGH_OFFSET 10 // 사다리꼴의 윗변 / 2
#define LOW_OFFSET 200 // 사다리꼴의 밑변 / 2
#define HIGH_POINT 180 // 사다리꼴의 윗 꼭짓점 좌표
#define LOW_POINT 300 // 사다리꼴의 아랫 꼭짓점 좌표

#include <opencv2/opencv.hpp>

#ifndef LATEST_INFO
#define LATEST_INFO

#include "../common/latestInfo.cpp"

#endif


class Roi {
public:
    std::vector<cv::Point> points;
    cv::Mat mask;
    LatestInfo latest;
    bool is_right;

    explicit Roi(bool is_right);

    void set_mask();

    void mask_reset();

    void calc_mask();
};

#endif //LEE_ROI_H
