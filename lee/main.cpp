#include <iostream>
#include <opencv2/opencv.hpp>

#include "../common/newTimer.cpp"

using namespace std;
using namespace cv;

#define DEBUG_MODE

#define WIDTH 640
#define HEIGHT 360

typedef Vec4i Line;

bool angle_filter(Line line, double angle_threshold) {
    double dx = abs(line[2] - line[0]);
    double dy = abs(line[3] - line[1]);
    double increse_rate = dy / dx;

    int w = WIDTH / 2;
    if (angle_threshold <= increse_rate && !(line[2] < (w / 2) && line[0] > (w / 2)) && !(line[2] > (w / 2) && line[0] < (w / 2)))
        return true;

    return false;
}

bool cross_filter(Line line) {
    int mid = WIDTH / 2;
    if ((line[0] > mid && line[2] > mid) || (line[0] < mid && line[2] < mid))
        return true;

    return false;
}

void info_mat(Mat mat) {
    cout << "width : " << mat.cols << endl;
    cout << "height : " << mat.rows << endl;
    cout << "channels : " << mat.channels() << endl;
}

Mat draw_lines(Mat frame, vector<Vec4i> lines) {
    Mat dst = frame.clone();
    for (Vec4i l : lines) {
        line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, LINE_AA);
    }
    return dst;
}

int main(int argc, char *argv[]) {
    VideoCapture cap;
    int w, h;
    Mat frame, resize_frame, gray_frame, mask;
    vector<Point> pts;
    TickMeter tm, tm2;
    Mat binarization, edge, dst, closed_edge;
    vector<Vec4i> lines, filtered_lines;
    double fps;
    string filepath = "./";
    string filename = "3.avi";

    TimeLapse tl = TimeLapse(8);
    int idx = 0;

    cap.open(filepath + filename, CAP_ANY);

    if (!cap.isOpened()) {
        cerr << " img read failed!" << endl;
        return -1;
    }

    w = WIDTH;
    h = HEIGHT;

    fps = cap.get(CAP_PROP_FPS);

    int delay = cvRound(1000 / fps);

    pts.push_back(Point(120, 300));
    pts.push_back(Point(310, 180));
    pts.push_back(Point(330, 180));
    pts.push_back(Point(520, 300));

    mask = Mat::zeros(h, w, CV_8UC1);
    fillPoly(mask, pts, Scalar(255), LINE_AA);
    mask = ~mask;

    while (true) {
#ifdef DEBUG_MODE
        tl.restart();
#endif

        cap >> frame;
        tl.prev_img = frame.clone();

        if (frame.empty())
            break;
        idx = 0;

#ifdef DEBUG_MODE
        tl.proc_record(frame);
#endif

        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);  // 3채널 -> 1채널

#ifdef DEBUG_MODE
        tl.proc_record(gray_frame);
#endif

        threshold(gray_frame, binarization, 120, 255, THRESH_BINARY);  // 이진화

#ifdef DEBUG_MODE
        tl.proc_record(binarization);
#endif

        binarization.setTo(Scalar(0), mask);  // 마스킹

#ifdef DEBUG_MODE
        tl.proc_record(binarization);
#endif

        Canny(binarization, edge, 50, 150, 7);  // 에지 검출

#ifdef DEBUG_MODE
        tl.proc_record(edge);
#endif

        HoughLinesP(edge, lines, 1, CV_PI / 180, 30, 100, 50);  // 직선 검출

#ifdef DEBUG_MODE
        if (lines.empty()) {
            // vi.undetected++;
        } else {
            dst = draw_lines(frame, lines);
        }

        tl.proc_record(dst);
#endif

        filtered_lines.clear();

        for (Vec4i l : lines) {
            if (angle_filter(l, 0.5)) {
                filtered_lines.push_back(l);
            }
        }

        dst = draw_lines(frame, filtered_lines);
#ifdef DEBUG_MODE
        tl.proc_record(dst);
#endif

        lines = filtered_lines;
        filtered_lines.clear();

        for (Vec4i l : lines) {
            if (cross_filter(l)) {
                filtered_lines.push_back(l);
            }
        }

        dst = draw_lines(frame, filtered_lines);

#ifdef DEBUG_MODE
        tl.proc_record(dst);
        tl.total_record(frame, dst);
#else
        imshow("original", frame);
        imshow("dst", dst);
        imshow("binarization", binarization);

        resizeWindow("original", w, h);
        resizeWindow("dst", w, h);
        resizeWindow("binarization", w, h);

        moveWindow("dst", w, 0);
        moveWindow("binarization", 0, h);

        if (waitKey(5)) {
        }
#endif
    }

#ifndef DEBUG_MODE
    destroyAllWindows();
#endif

#ifdef DEBUG_MODE
    tl.print_info_all();
    // cout << vi.undetected << endl;
#endif
    return 0;
}