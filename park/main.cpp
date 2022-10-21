#include <iostream>
#include <algorithm>
#include "opencv2/opencv.hpp"

#define TEST
#ifdef TEST

#include "../common/newTimer.cpp"

Video_info vi(6);

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
 */
void filterLines(InputOutputArray frame, const std::vector<Vec4i> &lines) {
    double left_max = 0, right_max = 0;
    std::vector<Vec4i> lane(2);

    for (Vec4i pts: lines) {
        Point p1(pts[0], pts[1]), p2(pts[2], pts[3]);
        // 직선 필터링... (임시)
        double m = getInclination(p1, p2);
        if (abs(m) < 0.3) {
            drawLines(frame, {pts}, Scalar(0, 0, 255));
            continue;
        }
        if (m > 0) {
            if (m > left_max) {
                left_max = m;
                lane[0] = pts;
            }
        } else {
            if (m < right_max) {
                right_max = m;
                lane[1] = pts;
            }
        }
        drawLines(frame, {pts});
    }
    drawLines(frame, lane, Scalar(255, 0, 0));
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
    int idx = 0;
#endif
    // 0. to grayscale
    Mat grayscaled;
    cvtColor(frame, grayscaled, COLOR_BGR2GRAY);
#ifdef TEST
    vi.proc_record(idx++, grayscaled);
//    Time_record(tm, vi, idx++, grayscaled);
#endif
    // 1. 주어진 임계값(default:130)으로 이진화
    threshold(grayscaled, grayscaled, 130, 145, THRESH_BINARY);

#ifdef TEST
    vi.proc_record(idx++, grayscaled);
//    Time_record(tm, vi, idx++, grayscaled);
#endif
//    showImage("grayscaled", grayscaled);

    // 2. apply roi
    Mat roi;
    applyStaticROI(grayscaled, roi);
#ifdef TEST
//    Time_record(tm, vi, idx++, roi);
    vi.proc_record(idx++, roi);
#endif
//    showImage("roi", roi);

    // 3. canny
    Mat edge;
    Canny(roi, edge, 50, 150);
#ifdef TEST
//    Time_record(tm, vi, idx++, edge);
    vi.proc_record(idx++, edge);
#endif

//    showImage("edge", edge);

    // 4. hough line
    std::vector<Vec4i> lines;
    HoughLinesP(edge, lines, 1, CV_PI / 180, 10, 100, 200);
#ifdef TEST
//    tm.stop();
    vi.stop_timer();
    Mat preview_lines = frame.getMat().clone();
    drawLines(preview_lines, lines);
//    Time_record(tm, vi, idx++, preview_lines);
    vi.proc_record(idx++, preview_lines);
#endif

    //5. filter lines
    Mat result = frame.getMat().clone();
    filterLines(result, lines);
#ifdef TEST
//    Time_record(tm, vi, idx++, result);
    vi.proc_record(idx++, result);
    //6. total
//    Time_record(tm2, vi, idx++, result);
    vi.total_record(result);
#endif
//    showImage("result", result, 10);
//    destroyAllWindows();
}

void videoHandler(const string &file_name, int tc) {
    VideoCapture video(file_name);

    if (!video.isOpened()) {
        cout << "Can't open video\n";
        return;
    }

    Mat frame;
//    Video_info vi;
    vi.set_tc(tc);

    while (true) {
        video >> frame;

        if (frame.empty()) {
            break;
        }
        vi.total_frame++;
        vi.prev_img = frame.clone();
        test(frame);
    }

    video.release();
    vi.print_info_all();
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
    glob(SRC_PREFIX + "2.avi", file_list);

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
        videoHandler(file_name, atoi(argv[1]));
    }

    return 0;
}
