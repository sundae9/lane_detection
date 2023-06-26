#include <iostream>
#include "opencv2/opencv.hpp"
#include "./ROI.hpp"

ROI roi;

#ifdef TIME_TEST

#include "../profile/timeLapse.hpp"

TimeLapse tl(6); // 시간 측정

#endif //TIME_TEST
#ifdef DETECTION_RATE
int detected[3], frame_cnt;
#endif //DETECTION_RATE
#ifdef VIDEO_SAVE

#include "../profile/oneVideoWriter.hpp"

OneVideoWriter vw;
#endif //VIDEO_SAVE
using namespace std;
using namespace cv;

const string SRC_PREFIX = "../../video/";

#ifdef SHOW

// 디버깅 용 이미지 출력 함수
void showImage(const string &label, InputArray img, int t = 0, int x = 0, int y = 0) {
    namedWindow(label, 1);
    moveWindow(label, x, y);
    imshow(label, img);
    waitKey(t);
}

#endif //SHOW

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
        line(frame, p1, p2, color, 1, LINE_AA);
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

        if (lane[i].grad != 0) {
            int x1 = roi.line_info[i].get_avg().coordX;
            int x2 = x1 - DEFAULT_ROI_HEIGHT * lane[i].grad;
            // ROI 기준 선 빨간색으로 표시
            line(frame, {x1, DEFAULT_ROI_DOWN}, {x2, DEFAULT_ROI_UP}, Scalar(0, 0, 255), 3, LINE_AA);
        }
    }

    int idx = -1, pos; // 선분 index, pos: 왼쪽 or 오른쪽
    double m;

    for (Vec4i pts: lines) {
        idx++;

        Point p1(pts[0], pts[1]), p2(pts[2], pts[3]);

        m = getCotangent(p1, p2);

        if (abs(m) > GRADIENT_STD) { // 기준 기울기보다 작은 경우 (역수로 계산하므로 부등호 반대)
#ifdef GRAPHIC
            line(frame, p1, p2, Scalar(0, 0, 255), 1, LINE_AA);
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
        line(frame, p1, p2, Scalar(0, 255, 0), 1, LINE_AA);
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

            line(frame, {x1, DEFAULT_ROI_DOWN}, {x2, DEFAULT_ROI_UP}, Scalar(255, 0, 0), 1, LINE_AA);
        } else {
            // 차선 검출 성공
            Point p1(lines[lane[i].idx][0], lines[lane[i].idx][1]), p2(lines[lane[i].idx][2], lines[lane[i].idx][3]);
            // 파란색으로 표기
            line(frame, p1, p2, Scalar(255, 0, 0), 1, LINE_AA);

            roi.line_info[i].update_lines({calculateX1(p1, p2), getCotangent(p1, p2)});
        }
    }
    roi.updateROI();

#ifdef DETECTION_RATE
    if (roi.adaptive_flag) { // 동적 roi - 어쨌든 차선 결정됨
        detected[2]++;
    } else if (lane[0].idx == -1 && lane[1].idx == -1) { // 0개 검출
        detected[0]++;
    } else if (lane[0].idx == -1 || lane[1].idx == -1) { // 1개 검출
        detected[1]++;
    } else {
        detected[2]++;
    }
#endif
}

void test(InputArray frame) {
#ifdef TIME_TEST
    tl.restart(); // 타이머 재설정
#endif //TIME_TEST
    // 0. to grayscale
    Mat grayscaled;
    cvtColor(frame, grayscaled, COLOR_BGR2GRAY);

#if defined(SHOW) || defined(VIDEO_SAVE)
    Mat show_roi = grayscaled.clone(); // roi 마스킹 화면 출력용
#endif //SHOW or VIDEO_SAVE
#ifdef TIME_TEST
    tl.proc_record(grayscaled); // 0. to grayscale
#endif // TIME_TEST

    // 1. 주어진 임계값(default:130)으로 이진화
    threshold(grayscaled, grayscaled, 130, 145, THRESH_BINARY);

#ifdef TIME_TEST
    tl.proc_record(grayscaled); // 1. 주어진 임계값(default:130)으로 이진화
#endif //TIME_TEST

    // 2. apply roi
    Mat roi_applied;
    roi.applyROI(grayscaled, roi_applied);

#if defined(SHOW) || defined(VIDEO_SAVE)
    roi.applyROI(show_roi, show_roi);

#ifdef SHOW
    showImage("roi", show_roi, 5);
#endif //SHOW
#ifdef VIDEO_SAVE
    vw.writeFrame(show_roi, 0);
#endif //VIDEO_SAVE

#ifdef SHOW
    showImage("threshold", roi_applied, 5, FRAME_WIDTH);
#endif //SHOW
#ifdef VIDEO_SAVE
    vw.writeFrame(roi_applied, 1);
#endif //VIDEO_SAVE

#endif //SHOW or VIDEO_SAVE

#ifdef TIME_TEST
    tl.proc_record(roi_applied); // 2. apply roi
#endif // TIME_TEST

    // 3. canny
    Mat edge;
    Canny(roi_applied, edge, 50, 150);

#ifdef SHOW
    showImage("edge", edge, 5, 0, FRAME_HEIGHT);
#endif // SHOW
#ifdef VIDEO_SAVE
    vw.writeFrame(edge, 2);
#endif //VIDEO_SAVE

#ifdef TIME_TEST
    tl.proc_record(edge); // 3. canny
#endif // TIME_TEST

    // 4. hough line
    std::vector <Vec4i> lines;
    HoughLinesP(edge, lines, 1, CV_PI / 180, 10, 100, 200);

#ifdef TIME_TEST
    tl.stop_both_timer();
    Mat preview_lines = frame.getMat().clone(); // 필터링 전 검출한 선분 그리기
    drawLines(preview_lines, lines);

    tl.proc_record(preview_lines); // 4. hough line
#endif //TIME_TEST

    // 5. filter lines
    Mat result = frame.getMat().clone();
    filterLinesWithAdaptiveROI(result, lines);

#ifdef SHOW
    showImage("result", result, 5, FRAME_WIDTH, FRAME_HEIGHT);
#endif // SHOW
#ifdef VIDEO_SAVE
    vw.writeFrame(result, 3);
#endif //VIDEO_SAVE
#ifdef TIME_TEST
    tl.proc_record(result); // 5. filter lines
    tl.total_record(frame.getMat(), result); // 6. total
#endif // TIME_TEST
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
#ifdef TIME_TEST
        tl.prev_img = frame.clone(); // 처리 전 원본 사진 저장
#endif //TIME_TEST
#ifdef DETECTION_RATE
        frame_cnt++;
#endif //DETECTION_RATE
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
#ifdef VIDEO_SAVE
    string dst_prefix = "../result/video/";
    string cmd = "mkdir " + dst_prefix + argv[1];
    system(cmd.c_str());
#endif
    for (string &file_name: file_list) {
#ifdef TIME_TEST
        tl = TimeLapse(6);
        tl.set_tc(1);
//        tl.set_tc(stoi(argv[1]));
#endif // TIME_TEST
#ifdef DETECTION_RATE
        for (int i = 0; i < 3; i++) {
            detected[i] = 0;
        }
        frame_cnt = 0;
#endif //DETECTION_RATE
#ifdef VIDEO_SAVE
        auto pos = file_name.rfind('.');
        string save_path = dst_prefix + argv[1] + "/" + file_name.substr(pos - 2, 2) + ".avi";
        vw = OneVideoWriter(save_path, FRAME_WIDTH, FRAME_HEIGHT, 2, 2, 4);
#endif //VIDEO_SAVE

        videoHandler(file_name);

#ifdef VIDEO_SAVE
        cout << "completed: " << save_path << '\n';
#endif //VIDEO_SAVE
#ifdef TIME_TEST
        tl.print_info_all();
#endif //TIME_TEST
#ifdef DETECTION_RATE
        for (int i = 0; i < 3; i++) {
            cout << detected[i] << ',';
        }
        cout << frame_cnt << '\n';
#endif //DETECTION_RATE
    }
    return 0;
}
