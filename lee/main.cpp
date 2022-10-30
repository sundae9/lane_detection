#include <iostream>
#include <opencv2/opencv.hpp>
#include <utility>

#include "../common/newTimer.cpp"
#include "../common/latestInfo.cpp"
#include "./Roi.cpp"

using namespace std;
using namespace cv;

#define DEBUG_MODE

#define WIDTH 640
#define HEIGHT 360

typedef Vec4i Line;
typedef vector<Line> Lines;


/**
 * 선의 기울기 반환
 * @param line 기울기를 구할 선
 * @return 기울기를 반환
 */
double increse_rate(Line line) {
    double dx = line[2] - line[0];
    double dy = line[3] - line[1];
    double increse_rate = dy / dx;

    return increse_rate;
}


bool angle_filter(Line line, double angle_threshold) {
    double grad = abs(increse_rate(line));

    int w = WIDTH / 2;
    if (angle_threshold <= grad && !(line[2] < (w / 2) && line[0] > (w / 2)) &&
        !(line[2] > (w / 2) && line[0] < (w / 2)))
        return true;

    return false;
}


bool cross_filter(Line line) {
    int mid = WIDTH / 2;
    if ((line[0] > mid && line[2] > mid) || (line[0] < mid && line[2] < mid))
        return true;

    return false;
}


Line most_angle_filter(Lines lines) {
    int max_angle = -1;
    Line most_line;
    double dx = 0, dy = 0, grad = 0;

    for (const Vec4i &l: lines) {
        grad = abs(increse_rate(l));

        if (grad > max_angle) {
            most_line = l;
            max_angle = grad;
        }
    }

    return most_line;
}


void info_mat(Mat mat) {
    cout << "width : " << mat.cols << endl;
    cout << "height : " << mat.rows << endl;
    cout << "channels : " << mat.channels() << endl;
}


Mat draw_lines(const Mat &frame, const Lines &lines) {
    Mat dst = frame.clone();
    for (Vec4i l: lines) {
        line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, LINE_AA);
    }
    return dst;
}

Mat draw_line(const Mat &frame, Line l) {
    Mat dst = frame.clone();
    line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, LINE_AA);
    return dst;
}

void show_image(Mat original, const Lines &lines) {
    Mat dst = draw_lines(original, lines);

    imshow("original", original);
    imshow("dst", dst);

    resizeWindow("original", WIDTH, HEIGHT);
    resizeWindow("dst", WIDTH, HEIGHT);

    moveWindow("dst", WIDTH, 0);

}

Mat adapt_mask(TimeLapse &tl, const Mat &frame, Mat mask) {
    Mat resize_frame, gray_frame, binarization;

#ifdef DEBUG_MODE
    tl.restart();
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

    return binarization;
}


Line detection(TimeLapse &tl, const Mat &original, const Mat &frame, LatestInfo latest) {
    Mat edge, dst;
    vector<Vec4i> lines, filtered_lines;

    Canny(frame, edge, 50, 150, 7);  // 에지 검출

#ifdef DEBUG_MODE
    tl.proc_record(edge);
#endif

    HoughLinesP(edge, lines, 1, CV_PI / 180, 30, 100, 50);  // 직선 검출

    dst = draw_lines(frame, lines);

#ifdef DEBUG_MODE
    tl.proc_record(dst);
#endif

    filtered_lines.clear();

    for (const Vec4i &l: lines) {
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

    for (const Vec4i &l: lines) {
        if (cross_filter(l)) {
            filtered_lines.push_back(l);
        }
    }

//    dst = draw_lines(original, filtered_lines);


#ifdef DEBUG_MODE
    tl.proc_record(dst);
#endif

    Line line = most_angle_filter(filtered_lines);

#ifdef DEBUG_MODE
    tl.proc_record(dst);
    tl.total_record(frame, dst);

#else
    //    show_image(original, filtered_lines);

    //    if (waitKey(5)) {
    //        waitKey();
    //    }
#endif

    return line;
}


int main(int argc, char *argv[]) {
    VideoCapture cap;
    vector<Point> pts;
    Mat original, frame, mask;
    Lines lines;

    string filepath = "../";
    string filename = "3.avi";

    LatestInfo latest;
    latest.reset();

    TimeLapse tl = TimeLapse(9);
    tl.set_tc(1);

    Roi left_mask(false), right_mask(true);

    cap.open(filepath + filename, CAP_ANY);

    if (!cap.isOpened()) {
        cerr << "img read failed!" << endl;
        return -1;
    }

    while (true) {
        cap >> original;

        if (original.empty())
            break;

        tl.prev_img = original.clone();
        frame = original.clone();

        frame = adapt_mask(tl, frame, left_mask.mask);
        lines.push_back(detection(tl, original, frame, latest));

        frame = original.clone();

        frame = adapt_mask(tl, frame, right_mask.mask);
        lines.push_back(detection(tl, original, frame, latest));

#ifndef DEBUG_MODE
        show_image(original, lines);
        if (waitKey(5)) {

        }
        lines.clear();
#endif


    }


#ifdef DEBUG_MODE
    tl.print_info_all();
#else
    destroyAllWindows();
#endif
    // cout << vi.undetected << endl;
    return 0;
}
