//
// Created by JiYoung Park on 2022/10/09.
//

#ifndef LEE_TIMELAPSE_HPP
#define LEE_TIMELAPSE_HPP

#endif //LEE_TIMELAPSE_HPP

#include <iostream>
#include <vector>
#include "opencv2/core/utility.hpp"

using namespace std;
using namespace cv;

// 시간 측정하는 클래스
class timeLapse {
private:
    int step_cnt;
    vector<string> title;
    vector<vector<double>> step_record;
    TickMeter tm_step;
    TickMeter tm_total;
public:
    timeLapse(vector<string> title) {
        this->title = title;
        this->step_cnt = title.size();
        this->title.push_back("total");
        this->step_record.resize(this->step_cnt + 1, vector<double>());
    }

    void setCheckpoint(int step);

    void stop(int step);

    void print();
};