#include <iostream>
#include <opencv2/opencv.hpp>
#include <utility>

#include "../common/newTimer.cpp"
#include "../common/latestInfo.cpp"
#include "./Roi.cpp"

using namespace std;
using namespace cv;

//#define DEBUG_MODE

#define WIDTH 640
#define HEIGHT 360

typedef Vec4i Line;
typedef vector<Line> Lines;


/**
 *
 * @param line 기울기를 구할 선
 * @return 기울기를 반환
 */
double increse_rate(Line line) {
    double dx = line[2] - line[0];
    double dy = line[3] - line[1];
    double increse_rate = dy / dx;

    return increse_rate;
}

/**
 *
 * @param line 선분
 * @param angle_threshold 기울기 임계치
 * @return 특정 각도(angle_threshold) 이상의 선분일 경우 true, 아니라면 false
 */
bool angle_filter(Line line, double angle_threshold) {
    double grad = abs(increse_rate(line));

    int w = WIDTH / 2;
    if (angle_threshold <= grad && !(line[2] < (w / 2) && line[0] > (w / 2)) &&
        !(line[2] > (w / 2) && line[0] < (w / 2)))
        return true;

    return false;
}

/**
 *
 * @param line 선분
 * @return 화면의 중앙을 넘는 선분이라면 false
 */
bool cross_filter(Line line) {
    int mid = WIDTH / 2;
    if ((line[0] > mid && line[2] > mid) || (line[0] < mid && line[2] < mid))
        return true;

    return false;
}

/**
 *
 * @param lines 선분들
 * @return 기울기가 가장 큰 선분 반환
 */
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

/**
 *
 * @param original 오리지날 행렬
 * @param lines 선분들
 * @return 선분들을 그린 행렬 반환
 */
Mat draw_lines(const Mat &original, const Lines &lines) {
    Mat dst = original.clone();
    for (Vec4i l: lines) {
        line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, LINE_AA);
    }
    return dst;
}

/**
 *
 * @param original 오리지날 행렬
 * @param l 선분
 * @return 선분을 그린 행렬 반환
 */
Mat draw_line(const Mat &original, Line l) {
    Mat dst = original.clone();
    line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, LINE_AA);
    return dst;
}

/**
 * @brief 오리지날 행렬에 선분을 그린 후 출력
 * @param original  오리지날 행렬
 * @param lines 선분들
 */
void show_image(Mat original, const Lines &lines) {
    Mat dst = draw_lines(original, lines);

    imshow("original", original);
    imshow("dst", dst);

    resizeWindow("original", WIDTH, HEIGHT);
    resizeWindow("dst", WIDTH, HEIGHT);

    moveWindow("dst", WIDTH, 0);

}

/***
 * @brief 오리지날 행렬에 마스킹 적용 후 반환
 * @param tl TimeLapse 객체
 * @param frame 오리지날 행렬
 * @param mask mask 행렬
 * @return mask를 적용시킨 행렬
 */
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

/**
 * @brief 마스킹 된 이미지를 받아 lane detection 처리
 * @param tl TimeLapse 객체
 * @param original 오리지날 행렬
 * @param frame 마스킹 적용한 행렬
 * @param latest LatestInfo 객체
 * @return 최종적으로 선정된 선분 반환
 */
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

    dst = draw_lines(original, filtered_lines);


#ifdef DEBUG_MODE
    tl.proc_record(dst);
#endif

    Line line = most_angle_filter(filtered_lines);
    dst = draw_line(original, line);

#ifdef DEBUG_MODE
    tl.proc_record(dst);
    tl.total_record(frame, dst);

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
