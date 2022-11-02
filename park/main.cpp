#include <iostream>
#include <algorithm>
#include "opencv2/opencv.hpp"
#include "./header/ROI.hpp"

ROI roi;

#ifdef DEBUG

#include "../common/newTimer.cpp"

TimeLapse tl(6);
#endif

using namespace std;
using namespace cv;

const string SRC_PREFIX = "../video/";

// 공통 함수
void showImage(const string &label, InputArray img, int t = 0, int x = 0, int y = 0) {
    namedWindow(label, 1);
    moveWindow(label, x, y);
    imshow(label, img);
    waitKey(t);
}

/**
 * 기울기 (역수) 구하는 함수 (cotangent)
 * @param p1 점 1
 * @param p2 점 2
 * @return 기울기
 */
double getCotangent(Point p1, Point p2) {
    return (double) (p1.x - p2.x) / (p1.y - p2.y);
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

int calculateX(Point p1, Point p2) {
    return (p1.x * (p2.y - DEFAULT_ROI_DOWN) - p2.x * (p1.y - DEFAULT_ROI_DOWN)) / (p2.y - p1.y);
}

/**
 * 허프 변환으로 검출한 차선 필터링
 * @param frame
 * @param lines
 */
void filterLinesWithAdaptiveROI(InputOutputArray frame, const std::vector<Vec4i> &lines) {
    struct Data {
        double grad, diff;
        int idx;
    };

    Data lane[2];

    // 초기화
    for (int i = 0; i < 2; i++) {
        lane[i].grad = roi.line_info[i].get_avg().gradient;
        lane[i].diff = 2.0;
        lane[i].idx = -1;
    }

    int idx = 0, pos;
    double m;

    for (Vec4i pts: lines) {
        Point p1(pts[0], pts[1]), p2(pts[2], pts[3]);
        m = getCotangent(p1, p2);
        if (abs(m) > GRADIENT_STD) {
            line(frame, p1, p2, Scalar(0, 0, 255), 1, 8);
            continue;
        }
        pos = m < 0 ? 0 : 1; // 왼쪽, 오른쪽 결정
        if (lane[pos].diff > abs(m - lane[pos].grad)) {
            lane[pos].diff = abs(m - lane[pos].grad);
            lane[pos].idx = idx;
        }
        drawLines(frame, {pts});
        idx++;
    }

    for (int i = 0; i < 2; i++) {
        if (lane[i].idx == -1) {
            Avg avg = roi.line_info[i].get_avg();
            int x1 = avg.coordX;
            int x2 = avg.coordX - (DEFAULT_ROI_DOWN - DEFAULT_ROI_UP) * avg.gradient;

            line(frame, {x1, DEFAULT_ROI_DOWN}, {x2, DEFAULT_ROI_UP}, Scalar(255, 0, 0), 1, 8);

            roi.line_info[i].not_found();
        } else {
            Point p1(lines[lane[i].idx][0], lines[lane[i].idx][1]), p2(lines[lane[i].idx][2], lines[lane[i].idx][3]);
            line(frame, p1, p2, Scalar(255, 0, 0), 1, 8);

            roi.line_info[i].update_lines({calculateX(p1, p2), getCotangent(p1, p2)});

        }
    }
    roi.updateROI();
}

/**
 * 허프 변환으로 검출된 선들을 필터링해서 그리는 함수
 * @param frame
 * @param lines
 * @return 검출된 라인 수
 */
void filterLines(InputOutputArray frame, const std::vector<Vec4i> &lines) {
    double left_max = -GRADIENT_STD, right_max = GRADIENT_STD;
    bool found[2] = {false, false};
    std::vector<Vec4i> lane(2);
    int pos; // 왼쪽 vs 오른쪽
    double m;
    for (Vec4i pts: lines) {
        Point p1(pts[0], pts[1]), p2(pts[2], pts[3]);
        // 직선 필터링... (임시)
        m = getCotangent(p1, p2);
        // 30도보다 작을 때
        if (abs(m) > GRADIENT_STD) {
            line(frame, p1, p2, Scalar(0, 0, 255), 1, 8);
            continue;
        }
        pos = m < 0 ? 0 : 1; // 왼쪽, 오른쪽 결정

        if (m < 0) { //left
            if (m > left_max) {
                left_max = m;
                lane[0] = pts;
                found[0] = true;
            }
        } else { // right
            if (m < right_max) {
                right_max = m;
                lane[1] = pts;
                found[1] = true;
            }
        }
        line(frame, p1, p2, Scalar(0, 255, 0), 1, 8);
    }
    drawLines(frame, lane, Scalar(255, 0, 0));
    for (int i = 0; i < 2; i++) {
        if (!found[i]) {
            roi.line_info[i].not_found();
            continue;
        }
        Point p1(lane[i][0], lane[i][1]), p2(lane[i][2], lane[i][3]);
        roi.line_info[i].update_lines({calculateX(p1, p2), getCotangent(p1, p2)});
    }
    roi.updateROI();

#ifdef DEBUG
    if (!found[0] && !found[1]) { // 둘 다 못 찾았을 경우
        roi.stat.zero_detected++;
    } else if (!found[0] || !found[1]) { // 둘 중 하나라도 못 찾았을 경우
        roi.stat.one_detected++;
    }
#endif
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
#ifdef DEBUG
    tl.restart();
#endif
    // 0. to grayscale
    Mat grayscaled;
    cvtColor(frame, grayscaled, COLOR_BGR2GRAY);
#ifdef DEBUG
    tl.proc_record(grayscaled);
#ifdef SHOW
    showImage("gray", grayscaled, 10);
#endif
#endif
    // 1. 주어진 임계값(default:130)으로 이진화
    threshold(grayscaled, grayscaled, 130, 145, THRESH_BINARY);
#ifdef DEBUG
    tl.proc_record(grayscaled);
#endif

    // 2. apply roi
    Mat roi_applied;
//    roi.a
    roi.applyROI(grayscaled, roi_applied);
#ifdef DEBUG
    tl.proc_record(roi_applied);
#ifdef SHOW
    showImage("roi", roi_applied, 10, FRAME_WIDTH);
#endif
//    cout << roi.adaptive_flag << '\n';

#endif

    // 3. canny
    Mat edge;
    Canny(roi_applied, edge, 50, 150);
#ifdef DEBUG
    tl.proc_record(edge);
    Mat tmp = edge.clone();
#ifdef SHOW
    showImage("edge", edge, 10, 0, FRAME_HEIGHT);
#endif
#endif
    // 4. hough line
    std::vector<Vec4i> lines;
    HoughLinesP(edge, lines, 1, CV_PI / 180, 10, 100, 200);
#ifdef DEBUG
    tl.stop_both_timer();
    Mat preview_lines = frame.getMat().clone();
    drawLines(preview_lines, lines);
    tl.proc_record(preview_lines);
#endif

    //5. filter lines
    Mat result = frame.getMat().clone();
    if (roi.adaptive_flag) {
        filterLinesWithAdaptiveROI(result, lines);
    } else {
        filterLines(result, lines);
    }

#ifdef DEBUG
    tl.proc_record(result);

    //6. total
    tl.total_record(frame.getMat(), result);

#ifdef SHOW
    showImage("result", result, 10, FRAME_WIDTH, FRAME_HEIGHT);
#endif
#endif
//    showImage("result", result, 10);
//    destroyAllWindows();
}

void videoHandler(const string &file_name) {
    VideoCapture video(file_name);
    roi = ROI();
    roi.initROI();

    if (!video.isOpened()) {
        cout << "Can't open video\n";
        return;
    }

    Mat frame;

    while (true) {
        video >> frame;

        if (frame.empty()) {
            break;
        }
#ifdef DEBUG
        tl.prev_img = frame.clone();
#endif
        test(frame);
    }

    video.release();
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

    for (string &file_name: file_list) {
//        imageHandler(file_name);
#ifdef DEBUG
        tl = TimeLapse(6);
        tl.set_tc(1);
#endif
        videoHandler(file_name);
#ifdef DEBUG
//        tl.print_info_all();
        //        cout << "static ROI\n";
        //        cout << file_name << " | " << roi.stat.staticROI << " | "
        //             << (double) roi.stat.staticROI / tl.total_frame * 100 << "%\n";
//        cout << "zero detected\n";
        cout << file_name << " | " << roi.stat.zero_detected << " | " << tl.total_frame << " | "
             << (double) roi.stat.zero_detected / tl.total_frame * 100 << "%\n";
#endif
    }
    return 0;
}
