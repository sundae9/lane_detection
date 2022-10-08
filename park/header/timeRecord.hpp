//
// Created by JiYoung Park on 2022/10/08.
//

#ifndef PARK_TIMERECORD_HPP
#define PARK_TIMERECORD_HPP

#include <iostream>
#include <vector>
#include "opencv2/core/utility.hpp"

using namespace std;
using namespace cv;

#endif //PARK_TIMERECORD_HPP

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