//
// Created by JiYoung Park on 2022/10/08.
//

#include "timeLapse.hpp"

void timeLapse::setCheckpoint(int step) {
    if (step > 0) {
        this->stop(step - 1);
    }
    if (step == 0) {
        this->tm_total.reset();
        this->tm_total.start();
    }
    this->tm_step.reset();
    this->tm_step.start();
}

void timeLapse::stop(int step) {
    this->tm_step.stop();
    this->step_record[step].push_back(this->tm_step.getTimeMilli());
    // 마지막 스텝 종료 후
    if (step + 1 == this->step_cnt) {
        tm_total.stop();
        this->step_record[this->step_cnt].push_back(this->tm_total.getTimeMilli());
    }
}

void timeLapse::print() {
    // print [step] [min] [max] [frame_idx]
    int min_idx, max_idx, cnt;
    double min, max, sum, tmp, avg;

    for (int i = 0; i < this->step_cnt + 1; i++) {
        min = INT32_MAX;
        max = -1;
        sum = 0;
        cnt = this->step_record[i].size();
        //        cout << title[i] << ' ';

        for (int j = 1; j < cnt; j++) {
            tmp = step_record[i][j];
            //            cout << tmp << ' ';
            sum += tmp;
            if (tmp < min) {
                min = tmp;
                min_idx = j;
            }
            if (tmp > max) {
                max = tmp;
                max_idx = j;
            }
        }
        //        cout << '\n';
        avg = sum / (double)(cnt - 1);
        cout << min << ", " << max << ", " << avg << ", " << min_idx << ", " << max_idx << '\n';
    }
}
