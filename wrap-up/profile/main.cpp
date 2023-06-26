#include "opencv2/opencv.hpp"
#include "./oneVideoWriter.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdlib.h>

using namespace cv;
using namespace std;

OneVideoWriter vw;

const string SRC_PREFIX = "../../";
const string DST_PREFIX = "../result/video/";

void makeVideo(string path1, string path2, string dst_path) {
    VideoCapture video[2];

    video[0] = VideoCapture(path1);
    video[1] = VideoCapture(path2);

    int width = video[0].get(CAP_PROP_FRAME_WIDTH);
    int height = video[0].get(CAP_PROP_FRAME_HEIGHT);

    vw = OneVideoWriter(dst_path, width, height, 1, 2, 2);

    if (!video[0].isOpened() || !video[0].isOpened()) {
        cout << "can't read video\n";
        return;
    }
    Mat frame;

    while (true) {
        for (int i = 0; i < 2; i++) {
            video[i] >> frame;
            if (frame.empty()) {
                video[0].release();
                video[1].release();
                return;
            }
            vw.writeFrame(frame, i);
        }
    }
}

int main(int argc, char *argv[]) {
    vector<string> file_list[2];

    for (int i = 0; i < 2; i++) {
        glob(SRC_PREFIX + argv[2 * i + 1] + "/result/video/" + argv[2 * i + 2] + "/*.avi", file_list[i]);
        sort(file_list[i].begin(), file_list[i].end());
    }

    if (file_list[0].size() != file_list[1].size()) {
        return 0;
    }
    string result_path;
    string cmd = "mkdir " + DST_PREFIX + argv[5];
    system(cmd.c_str());
    for (int i = 0; i < file_list[0].size(); i++) {

        auto pos = file_list[0][i].rfind('.');
        result_path = DST_PREFIX + argv[5] + "/" + file_list[0][i].substr(pos - 2, 2) + ".avi";

        makeVideo(file_list[0][i], file_list[1][i], result_path);
        cout << "completed: " << result_path << '\n';
    }

    return 0;
}