#ifndef LEE_ROI_H
#define LEE_ROI_H

#define WIDTH 640
#define HEIGHT 360


#include <opencv2/opencv.hpp>


class Roi {
public:
    std::vector<cv::Point> points;
    cv::Mat mask;
    bool is_right;

    explicit Roi(bool is_right);

    void set_points(std::vector<cv::Point> pts);

    void set_mask();
};

#endif //LEE_ROI_H
