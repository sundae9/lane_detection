#include <iostream>
#include "opencv2/opencv.hpp"
#include "./header/ROI.hpp"

ROI roi;

#ifdef DEBUG

#include "../common/newTimer.cpp"

TimeLapse tl(6); // 시간 측정

#endif

using namespace std;
using namespace cv;

const string SRC_PREFIX = "../video/";

// 디버깅 용 이미지 출력 함수
void showImage(const string &label, InputArray img, int t = 0, int x = 0, int y = 0) {
    namedWindow(label, 1);
    moveWindow(label, x, y);
    imshow(label, img);
    waitKey(t);
}

/**
 * 기울기 역수 구하는 함수 (cotangent)
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
void drawLines(InputOutputArray frame, const std::vector <Vec4i> &lines, Scalar color = Scalar(0, 255, 0)) {
    for (Vec4i pts: lines) {
        Point p1(pts[0], pts[1]), p2(pts[2], pts[3]);
        line(frame, p1, p2, color, 1, 8);
    }
}

/**
 * x1 좌표 계산
 * @param p1
 * @param p2
 * @return 검출 선분 & roi 밑변 교차점 - x 좌표
 */
int calculateX1(Point p1, Point p2) {
    return (p1.x * (p2.y - DEFAULT_ROI_DOWN) - p2.x * (p1.y - DEFAULT_ROI_DOWN)) / (p2.y - p1.y);
}

/**
 * 허프 변환으로 검출한 차선 필터링
 * @param frame
 * @param lines
 */
void filterLinesWithAdaptiveROI(InputOutputArray frame, const std::vector <Vec4i> &lines) {
    struct Data {
        double grad, diff;
        int idx;
    };

    Data lane[2]; // 왼쪽[0], 오른쪽[1] 차선

    // 초기화
    for (int i = 0; i < 2; i++) {
        lane[i].grad = roi.adaptive_flag ? roi.line_info[i].get_avg().gradient : 0;
        lane[i].diff = 2.0;
        lane[i].idx = -1;
    }

    int idx = -1, pos; // 선분 index, pos: 왼쪽 or 오른쪽
    double m;

    for (Vec4i pts: lines) {
        idx++;

        Point p1(pts[0], pts[1]), p2(pts[2], pts[3]);

        m = getCotangent(p1, p2);

        if (abs(m) > GRADIENT_STD) { // 기준 기울기보다 작은 경우 (역수로 계산하므로 부등호 반대)
#ifdef GRAPHIC
            line(frame, p1, p2, Scalar(0, 0, 255), 1, 8);
#endif //GRAPHIC
            continue;
        }

        pos = m < 0 ? 0 : 1; // 왼쪽, 오른쪽 결정

        // 기존 값과 차이가 최소인 선분 필터링
        if (abs(lane[pos].grad - m) < lane[pos].diff) {
            lane[pos].diff = abs(lane[pos].grad - m);
            lane[pos].idx = idx;
        }

        // 프레임에 표기 (붉은색)
        line(frame, p1, p2, Scalar(0, 255, 0), 1, 8);
    }

    // 검출한 선분 update
    for (int i = 0; i < 2; i++) {
        if (lane[i].idx == -1) {
            roi.line_info[i].not_found();

            // 동적 roi...
            if (!roi.adaptive_flag) {
                continue;
            }

            // 기존 값으로 차로 표기
            Avg avg = roi.line_info[i].get_avg();
            int x1 = avg.coordX;
            int x2 = avg.coordX - (DEFAULT_ROI_HEIGHT) * avg.gradient;

            line(frame, {x1, DEFAULT_ROI_DOWN}, {x2, DEFAULT_ROI_UP}, Scalar(255, 0, 0), 1, 8);
        } else {
            // 차선 검출 성공
            Point p1(lines[lane[i].idx][0], lines[lane[i].idx][1]), p2(lines[lane[i].idx][2], lines[lane[i].idx][3]);
            // 파란색으로 표기
            line(frame, p1, p2, Scalar(255, 0, 0), 1, 8);

            roi.line_info[i].update_lines({calculateX1(p1, p2), getCotangent(p1, p2)});
        }
    }
    roi.updateROI();

#ifdef DEBUG
    if (roi.adaptive_flag) { // 동적 roi - 어쨌든 차선 결정됨
        return;
    }
    if (lane[0].idx != -1 && lane[1].idx != -1) { // 둘 다 못 찾았을 경우
        roi.stat.zero_detected++;
    } else if (lane[0].idx != -1 || lane[1].idx != -1) { // 둘 중 하나라도 못 찾았을 경우
        roi.stat.one_detected++;
    }
#endif
}

void test(InputArray frame) {
#ifdef DEBUG
    tl.restart(); // 타이머 재설정
#endif
    // 0. to grayscale
    Mat grayscaled;
    cvtColor(frame, grayscaled, COLOR_BGR2GRAY);

#ifdef SHOW

#ifdef DEBUG
    tl.stop_both_timer();
#endif // DEBUG

    showImage("gray", grayscaled, 5);
    Mat show_roi = grayscaled.clone(); // roi 마스킹 화면 출력용
#endif // SHOW

#ifdef DEBUG
    tl.proc_record(grayscaled); // 0. to grayscale
#endif // DEBUG

    // 1. 주어진 임계값(default:130)으로 이진화
    threshold(grayscaled, grayscaled, 130, 145, THRESH_BINARY);

#ifdef DEBUG
    tl.proc_record(grayscaled); // 1. 주어진 임계값(default:130)으로 이진화
#endif

    // 2. apply roi
    Mat roi_applied;
    roi.applyROI(grayscaled, roi_applied);

#ifdef SHOW

#ifdef DEBUG
    tl.stop_both_timer();
#endif // DEBUG

    roi.applyROI(show_roi, show_roi); // roi 화면 출력용
    showImage("roi", show_roi, 5, FRAME_WIDTH);
#endif // SHOW


#ifdef DEBUG
    tl.proc_record(roi_applied); // 2. apply roi
#endif // DEBUG

    // 3. canny
    Mat edge;
    Canny(roi_applied, edge, 50, 150);

#ifdef SHOW

#ifdef DEBUG
    tl.stop_both_timer();
#endif

    showImage("edge", edge, 5, 0, FRAME_HEIGHT);
#endif // SHOW

#ifdef DEBUG
    tl.proc_record(edge); // 3. canny
#endif // DEBUG

    // 4. hough line
    std::vector <Vec4i> lines;
    HoughLinesP(edge, lines, 1, CV_PI / 180, 10, 100, 200);

#ifdef DEBUG
    tl.stop_both_timer();
    Mat preview_lines = frame.getMat().clone(); // 필터링 전 검출한 선분 그리기
    drawLines(preview_lines, lines);

    tl.proc_record(preview_lines); // 4. hough line
#endif

    // 5. filter lines
    Mat result = frame.getMat().clone();
    filterLinesWithAdaptiveROI(result, lines);

#ifdef SHOW

#ifdef DEBUG
    tl.stop_both_timer();
#endif // DEBUG

    showImage("result", result, 5, FRAME_WIDTH, FRAME_HEIGHT);
#endif // SHOW

#ifdef DEBUG
    tl.proc_record(result); // 5. filter lines
    tl.total_record(frame.getMat(), result); // 6. total
#endif // DEBUG
}

void videoHandler(const string &file_name) {
    VideoCapture video(file_name);

    if (!video.isOpened()) {
        cout << "Can't open video\n";
        return;
    }

    roi = ROI();
    roi.initROI();

    Mat frame;

    while (true) {
        video >> frame;

        if (frame.empty()) {
            break;
        }
#ifdef DEBUG
        tl.prev_img = frame.clone(); // 처리 전 원본 사진 저장
#endif //DEBUG
        test(frame);
    }

    video.release();
}


int main(int argc, char *argv[]) {
    vector <string> file_list;
    glob(SRC_PREFIX + "*.avi", file_list);

    if (file_list.empty()) {
        cout << "can't find image list\n";
        return 1;
    }

    for (string &file_name: file_list) {
#ifdef DEBUG
        tl = TimeLapse(6);
        tl.set_tc(1);
//        tl.set_tc(stoi(argv[1]));
#endif // DEBUG

        videoHandler(file_name);

#ifdef DEBUG
        tl.print_info_all();
        //        cout << "static ROI\n";
//        cout << file_name << " | " << roi.stat.staticROI << " | "
//             << (double) roi.stat.staticROI / tl.total_frame * 100 << "%\n";
//        cout << "zero detected\n";
//        cout << file_name << "," << roi.stat.zero_detected << "," << tl.total_frame << ","
//             << (double) roi.stat.zero_detected / tl.total_frame * 100 << "%\n";
#endif
    }
    return 0;
}
