#include <iostream>
#include <algorithm>
#include "opencv2/opencv.hpp"

#define TEST

#include "../common/newTimer.cpp"

TimeLapse tl(6);
#ifdef TEST

#endif

using namespace std;
using namespace cv;

const string SRC_PREFIX = "../video/";

vector<Point> roi_polygon(4);

// 공통 함수
void showImage(const string &label, InputArray img, int t = 0) {
    namedWindow(label, 1);
    imshow(label, img);
    waitKey(t);
}

// 기울기 => 차선 필터링 할 때 활용
double getInclination(Point2f p1, Point2f p2) {
    if (p1.x == p2.x) return 1e5;
    return (double) (p1.y - p2.y) / (p1.x - p2.x);
}

double getCotangent(Point p1, Point p2) {
    return (double) (p1.x - p2.x) / (p1.y - p2.y);
}


// 전처리 단계
// 1. 이진화
void thresholdDefault(InputArray frame, OutputArray result, int thresh = 130) {
    // 1. grayscale로 변환
    cvtColor(frame, result, COLOR_BGR2GRAY);
    // 2. 주어진 임계값(default:130)으로 이진화
    threshold(result, result, thresh, 145, THRESH_BINARY);
}

// 2. ROI 설정
void applyStaticROI(InputArray frame, OutputArray result) {
    // 사다리꼴 영역 설정
    int row = frame.rows(), col = frame.cols();
    Mat roi = Mat::zeros(row, col, CV_8U);
    fillPoly(roi, roi_polygon, 255);
    bitwise_and(frame, roi, result);
}

/**
 * 프레임에 선분 그리는 함수
 * @param frame 배경 프레임 (원본)
 * @param lines 그릴 선분들
 * @param color 색
 */
void drawLines(InputOutputArray frame, const std::vector<Vec4i> &lines, Scalar color = Scalar(0, 255, 0)) {
    for (Vec4i pts: lines) {
        Point p1(pts[0], pts[1]), p2(pts[2], pts[3]);
        line(frame, p1, p2, color, 1, 8);
    }
}

/**
 * 허프 변환으로 검출된 선들을 필터링해서 그리는 함수
 * @param frame
 * @param lines
 * @return 검출된 라인 수
 */
bool filterLines(InputOutputArray frame, const std::vector<Vec4i> &lines) {
    double left_max = 0, right_max = 0;
    bool left = false, right = false;
    std::vector<Vec4i> lane(2);

    for (Vec4i pts: lines) {
        Point p1(pts[0], pts[1]), p2(pts[2], pts[3]);
        // 직선 필터링... (임시)
        double m = getCotangent(p1, p2);
        if (abs(m) > 1.73) {
            drawLines(frame, {pts}, Scalar(0, 0, 255));
            continue;
        }
        if (m > 0) {
            if (m > left_max) {
                left_max = m;
                lane[0] = pts;
                left = true;
            }
        } else {
            if (m < right_max) {
                right_max = m;
                lane[1] = pts;
                right = true;
            }
        }
        drawLines(frame, {pts});
    }
    drawLines(frame, lane, Scalar(255, 0, 0));
    return !(left | right);
}

void houghLineSegments(InputArray frame, InputOutputArray result) {
    Mat edge;
    Canny(frame, edge, 50, 150);
//    showImage("edge", edge);
    std::vector<Vec4i> lines;
    HoughLinesP(edge, lines, 1, CV_PI / 180, 0, 100, 200);
    drawLines(result, lines);
}

void test(InputArray frame) {
#ifdef TEST
    tl.restart();
#endif
    // 0. to grayscale
    Mat grayscaled;
    cvtColor(frame, grayscaled, COLOR_BGR2GRAY);
#ifdef TEST
    tl.proc_record(grayscaled);
#endif
    // 1. 주어진 임계값(default:130)으로 이진화
    threshold(grayscaled, grayscaled, 130, 145, THRESH_BINARY);

#ifdef TEST
    tl.proc_record(grayscaled);
#endif
//    showImage("grayscaled", grayscaled);

    // 2. apply roi
    Mat roi;
    applyStaticROI(grayscaled, roi);
#ifdef TEST
    tl.proc_record(roi);
#endif
//    showImage("roi", roi);

    // 3. canny
    Mat edge;
    Canny(roi, edge, 50, 150);
#ifdef TEST
    tl.proc_record(edge);
#endif

//    showImage("edge", edge);

    // 4. hough line
    std::vector<Vec4i> lines;
    HoughLinesP(edge, lines, 1, CV_PI / 180, 10, 100, 200);
#ifdef TEST
    tl.stop_both_timer();
    Mat preview_lines = frame.getMat().clone();
    drawLines(preview_lines, lines);
    tl.proc_record(preview_lines);
#endif

    //5. filter lines
    Mat result = frame.getMat().clone();
#ifdef TEST
    tl.proc_record(result);
    //6. total
    tl.total_record(frame.getMat(), result);

#endif
    showImage("result", result, 10);
//    destroyAllWindows();
}

void videoHandler(const string &file_name) {
    VideoCapture video(file_name);

    if (!video.isOpened()) {
        cout << "Can't open video\n";
        return;
    }

    Mat frame;
    tl = TimeLapse(6);
    tl.set_tc(1);

    while (true) {
        video >> frame;

        if (frame.empty()) {
            break;
        }
        tl.total_frame++;
        tl.prev_img = frame.clone();
        test(frame);
    }

    video.release();
    tl.print_info_all();
}

void imageHandler(const string &file_name) {
    Mat frame = imread(file_name);
    if (frame.empty()) {
        cout << "Can't open image\n";
        return;
    }

//    test(frame);
//    destroyAllWindows();
}

int main(int argc, char *argv[]) {
    vector<string> file_list;
    glob(SRC_PREFIX + "*.avi", file_list);

    if (file_list.empty()) {
        cout << "can't find image list\n";
        return 1;
    }

    roi_polygon = {{275, 195},
                   {187, 284},
                   {461, 284},
                   {342, 195}};

    for (string &file_name: file_list) {
//        imageHandler(file_name);
        videoHandler(file_name);
    }

    return 0;
}
