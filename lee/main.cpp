#include <iostream>
#include <opencv2/opencv.hpp>

#include "../common/newTimer.cpp"

#ifndef LATEST_INFO

#define LATEST_INFO

#include "../common/latestInfo.cpp"

#endif

#include "./Roi.cpp"

using namespace std;
using namespace cv;

// DEBUG FLAG
#define DEBUG_MODE

// SHOW FLAG
//#define SHOW_MODE

#define WIDTH 640
#define HEIGHT 360

typedef Vec4i Line;
typedef vector<Line> Lines;

Line non_line = {0, 0, 0, 0};

/**
 *
 * @param line 기울기를 구할 선
 * @return 기울기를 반환
 */
double increse_rate(Line line) {
    double dx, dy;

    dx = line[2] - line[0];
    dy = line[3] - line[1];

    double grad = dy / dx;

    return grad;
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
 * @return 화면의 중앙을 지나는 선분이라면 false
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
void show_image(Mat original, const Lines &lines, Mat mask) {
    Mat dst = draw_lines(original, lines);
    Mat roi = original.clone();

    roi.setTo(Scalar(0), mask);

    imshow("original", original);
    imshow("dst", dst);
    imshow("roi", roi);

    resizeWindow("original", WIDTH, HEIGHT);
    resizeWindow("dst", WIDTH, HEIGHT);

    moveWindow("dst", WIDTH, 0);
    moveWindow("roi", WIDTH, HEIGHT);
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

/**
 * @brief 라인 정보를 입력받아, 사다리꼴의 밑변에 접하는 좌표와 기울기를 Line_info 구조체로 바꾸어 반환
 * @param line 라인 정보, 빈 라인이라면 {0,0,0,0}
 * @return Line_info 구조체
 */
Line_info info_line(Line line) {
    Line_info line_info;

    line_info.gradient = increse_rate(line);
    // (x1, y1) 을 지나고 기울기가 grad인 선이 사다리꼴 밑변의 접하는 점의 x좌표는
    // (grad*x1
    if (line[1] > line[3]) {
        line_info.coordX = (line_info.gradient * line[0] - line[1] + 300) / line_info.gradient;
    } else {
        line_info.coordX = (line_info.gradient * line[2] - line[3] + 300) / line_info.gradient;
    }

    return line_info;
}

void check_valid_line(const Line &line, Roi &roi) {
    if (line == non_line) {  // 라인이 존재하지 않음
        roi.latest.not_found();
        return;
    }

    Line_info line_info = info_line(line);

    if (roi.is_right) {
        if (line_info.gradient > 0) line_info.gradient = line_info.gradient * -1;
    } else if (line_info.gradient < 0)
        line_info.gradient = line_info.gradient * -1;

    roi.latest.update_lines(line_info);
}

int main(int argc, char *argv[]) {
    VideoCapture cap;
    vector<Point> pts;
    Mat original, frame, mask;

    Lines lines;
    Line line;

    string filepath = "./";
    string filename = to_string(atoi(argv[1]) + 1) + ".avi";

    TimeLapse tl = TimeLapse(9);
    TimeLapse tl2 = TimeLapse(9);

    tl.set_tc(atoi(argv[1]));
    tl2.set_tc(atoi(argv[1]));

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


        // 좌측 ROI 마스킹 및 디텍팅 후 큐에 저장
        left_mask.set_mask();
        tl.restart();

        frame = adapt_mask(tl, frame, left_mask.mask);
        line = detection(tl, original, frame, left_mask.latest);
        lines.push_back(line);

        check_valid_line(line, left_mask);

        // 우측 ROI 마스킹 및 디텍팅 후 큐에 저장
        frame = original.clone();

        right_mask.set_mask();
        tl2.restart();
        frame = adapt_mask(tl2, frame, right_mask.mask);
        line = detection(tl2, original, frame, right_mask.latest);
        lines.push_back(line);

        check_valid_line(line, right_mask);

        //        left_mask.latest.print_all();
        //        right_mask.latest.print_all();

        //        if (left_mask.latest.adaptive_ROI_flag && right_mask.latest.adaptive_ROI_flag) {
        //            waitKey();
        //        }

        Mat mask_area;
        bitwise_and(left_mask.mask, right_mask.mask, mask_area);
#ifdef SHOW_MODE
        show_image(original, lines, mask_area);
        if (waitKey(5)) {
            //            waitKey();
        }
#endif
        lines.clear();
    }

#ifdef DEBUG_MODE
    tl.print_info_all();
    cout << "\n\n";
    tl2.print_info_all();
#else
    destroyAllWindows();
#endif
    // cout << vi.undetected << endl;
    return 0;
}
