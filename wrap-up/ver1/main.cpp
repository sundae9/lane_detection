#include <iostream>
#include "opencv2/opencv.hpp"
#include "./constant.hpp"

using namespace std;
using namespace cv;

#ifdef TIMETEST

#include "../profile/timeLapse.hpp"

TimeLapse tl(6);
#endif //TIMETEST

#ifdef DETECTION_RATE
int detected[3], total;
#endif //DETECTION_RATE

#ifdef VIDEO_SAVE

#include "../profile/oneVideoWriter.hpp"

OneVideoWriter vw;
#endif //VIDEO_SAVE

const string SRC_PREFIX = "../../video/";

vector <Point> roi_polygon(4);

// 공통 함수
void showImage(const string &label, InputArray img, int t = 0) {
    namedWindow(label, 1);
    imshow(label, img);
    waitKey(t);
}

// 기울기 => 차선 필터링 할 때 활용
double getInclination(Point2f p1, Point2f p2) {
    if (p1.x == p2.x) return 0; // x좌표가 같은 경우(직각)는 검출된 선분 무시
    return (double) (p1.y - p2.y) / (p1.x - p2.x);
}

/**
 * ROi 적용하는 함수
 * @param frame 이진화 된 프레임
 * @param result roi 적용된 프레임
 */
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
void drawLines(InputOutputArray frame, const std::vector <Vec4i> &lines, Scalar color = Scalar(0, 255, 0)) {
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
void filterLines(InputOutputArray frame, const std::vector <Vec4i> &lines) {
    double left_max = 0, right_max = 0;
    std::vector <Vec4i> lane(2);
#ifdef DETECTION_RATE
    bool left = false, right = false;
#endif //DETECTION_RATE

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
#ifdef DETECTION_RATE
                left = true;
#endif //DETECTION_RATE
            }
        } else {
            if (m < right_max) {
                right_max = m;
                lane[1] = pts;
#ifdef DETECTION_RATE
                right = true;
#endif //DETECTION_RATE
            }
        }
        drawLines(frame, {pts});
    }
    drawLines(frame, lane, Scalar(255, 0, 0));
#ifdef DETECTION_RATE
    if (left && right) detected[2]++;
    else if (left || right) detected[1]++;
    else detected[0]++;
#endif //DETECTION_RATE
}

void test(InputArray frame) {
#ifdef TIMETEST
    tl.restart(); // 타이머 재시작
#endif //TIMETEST

    // 0. to grayscale
    Mat grayscaled;
    cvtColor(frame, grayscaled, COLOR_BGR2GRAY);
#ifdef TIMETEST
    tl.proc_record(grayscaled); // 0. grayscale
#endif //TIMETEST
#ifdef VIDEO_SAVE
    vw.writeFrame(grayscaled, 0);
#endif //VIDEO_SAVE

    // 1. 주어진 임계값(default:130)으로 이진화
    threshold(grayscaled, grayscaled, 130, 145, THRESH_BINARY);

//    showImage("grayscaled", grayscaled);
#ifdef TIMETEST
    tl.proc_record(grayscaled); // 1. threshold
#endif //TIMETEST

    // 2. apply roi
    Mat roi;
    applyStaticROI(grayscaled, roi);
//    showImage("roi", roi);
#ifdef TIMETEST
    tl.proc_record(roi); // 2. apply roi
#endif //TIMETEST
#ifdef VIDEO_SAVE
    vw.writeFrame(roi, 1);
#endif //VIDEO_SAVE

    // 3. canny
    Mat edge;
    Canny(roi, edge, 50, 150);
//    showImage("edge", edge);
#ifdef TIMETEST
    tl.proc_record(edge); // 3. canny edge
#endif //TIMETEST
#ifdef VIDEO_SAVE
    vw.writeFrame(edge, 2);
#endif //VIDEO_SAVE

    // 4. hough line
    std::vector <Vec4i> lines;
    HoughLinesP(edge, lines, 1, CV_PI / 180, 10, 100, 200);
#ifdef TIMETEST
    tl.stop_both_timer();
    Mat preview_lines = frame.getMat().clone();
    drawLines(preview_lines, lines);
    tl.proc_record(preview_lines); // 4. hough transformation
#endif //TIMETEST

    //5. filter lines
    Mat result = frame.getMat().clone();
    filterLines(result, lines);

#ifdef TIMETEST
    tl.proc_record(result); // 5. filter line
    tl.total_record(frame.getMat(), result); // 6. total
#endif //TIMETEST
#ifdef VIDEO_SAVE
    vw.writeFrame(result, 3);
#endif //VIDEO_SAVE
}

void videoHandler(const string &file_name) {
    VideoCapture video(file_name);

    if (!video.isOpened()) {
        cout << "Can't open video\n";
        return;
    }

#ifdef TIMETEST
    tl = TimeLapse(6);
    tl.set_tc(1);
#endif //TIMETEST

#ifdef DETECTION_RATE
    for (int i = 0; i < 3; i++) {
        detected[i] = 0;
    }
    total = 0;
#endif //DETECTION_RATE
#ifdef VIDEO_SAVE
    auto pos = file_name.rfind('.');
    string save_path = "../result/video/" + file_name.substr(pos - 2, 2) + ".mp4";
    vw = OneVideoWriter(save_path, FRAME_WIDTH, FRAME_HEIGHT, 2, 2, 4);
#endif // VIDEO_SAVE
    Mat frame;

    while (true) {
        video >> frame;

        if (frame.empty()) {
            break;
        }
#ifdef TIMETEST
        tl.prev_img = frame.clone();
#endif // TIMETEST
#ifdef DETECTION_RATE
        total++;
#endif //DETECTION_RATE

        test(frame);
    }
    video.release();

#ifdef TIMETEST
    tl.print_info_all();
#endif //TIMETEST
#ifdef DETECTION_RATE
    for (int i = 0; i < 3; i++) {
        cout << detected[i] << ',';
    }
    cout << total << '\n';
#endif //DETECTION_RATE
}

int main(int argc, char *argv[]) {
    vector <string> file_list;
    glob(SRC_PREFIX + "*.avi", file_list);

    if (file_list.empty()) {
        cout << "can't find video list\n";
        return 1;
    }

    roi_polygon = {{275, 195},
                   {187, 284},
                   {461, 284},
                   {342, 195}};

    for (string &file_name: file_list) {
        videoHandler(file_name);
    }

    return 0;
}
