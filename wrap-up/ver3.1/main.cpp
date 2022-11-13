#include <iostream>
#include "opencv2/opencv.hpp"
#include "./ROI.hpp"
#include "./adaptiveThresh.hpp"

ROI roi;
AdaptiveThresh adaptiveThresh;

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
        line(frame, p1, p2, color, 1, 8);
    }
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
        lane[i].grad = roi.line_info[i].adaptive_ROI_flag ? roi.line_info[i].line.gradient : 0;
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
            if (!roi.line_info[i].adaptive_ROI_flag) {
                continue;
            }

            // 기존 값으로 차로 표기
            Line_info prev = roi.line_info[i].line;
            line(frame, {prev.x_bottom, DEFAULT_ROI_HEIGHT}, {prev.x_top, 0}, Scalar(255, 0, 0), 1, 8);
        } else {
            // 차선 검출 성공
            Point p1(lines[lane[i].idx][0], lines[lane[i].idx][1]), p2(lines[lane[i].idx][2], lines[lane[i].idx][3]);
            // 파란색으로 표기
            line(frame, p1, p2, Scalar(255, 0, 0), 1, 8);
            roi.line_info[i].update_lines(p1, p2, getCotangent(p1, p2));
        }
    }
#ifdef DETECTION_RATE
    // 검출 -> 동적 roi가 작동 중이거나 혹은 idx =! -1 인 경우
    // 미검출 -> 동적 roi가 미작동이면서 idx==-1 인 경우
    bool left = roi.line_info[0].adaptive_ROI_flag || (lane[0].idx != -1);
    bool right = roi.line_info[1].adaptive_ROI_flag || (lane[1].idx != -1);

    if (left && right) detected[2]++;
    else if (left || right) detected[1]++;
    else detected[0]++;

    frame_cnt++;
#endif //DETECTION_RATE
    roi.updateROI();
}

void test(InputArray frame) {
#ifdef TIME_TEST
    tl.restart(); // 타이머 재설정
#endif //TIME_TEST
    Mat road_area = frame.getMat()(Range(DEFAULT_ROI_UP, DEFAULT_ROI_DOWN), Range(DEFAULT_ROI_LEFT, DEFAULT_ROI_RIGHT));
    // 0. to grayscale
    Mat grayscaled;
    cvtColor(road_area, grayscaled, COLOR_BGR2GRAY);

#ifdef SHOW
    showImage("gray", grayscaled, 5);
    Mat show_roi = grayscaled.clone(); // roi 마스킹 화면 출력용
#endif // SHOW
#ifdef VIDEO_SAVE
    vw.writeFrame(grayscaled, 0);
    Mat show_roi = grayscaled.clone(); // roi 마스킹 화면 출력용
#endif //VIDEO_SAVE

#ifdef TIME_TEST
    tl.proc_record(grayscaled); // 0. to grayscale
#endif // TIME_TEST

    // 1. 주어진 임계값(default:130)으로 이진화
    adaptiveThresh.applyThresholding(grayscaled, grayscaled);

#ifdef TIME_TEST
    tl.proc_record(grayscaled); // 1. 주어진 임계값(default:130)으로 이진화
#endif //TIME_TEST

    // 2. apply roi
    Mat roi_applied;
    roi.applyROI(grayscaled, roi_applied);
    int white = adaptiveThresh.updateThresholding(roi_applied);

#ifdef SHOW
    roi.applyROI(show_roi, show_roi); // roi 화면 출력용
    showImage("roi", roi_applied, 5, FRAME_WIDTH);
#endif // SHOW
#ifdef VIDEO_SAVE
    roi.applyROI(show_roi, show_roi);
    vw.writeFrame(roi_applied, 1);
#endif //VIDEO_SAVE

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
    HoughLinesP(edge, lines, 1, CV_PI / 180, 30, 40, 40);

#ifdef TIME_TEST
    tl.stop_both_timer();
    Mat preview_lines = frame.getMat().clone(); // 필터링 전 검출한 선분 그리기
    drawLines(preview_lines, lines);

    tl.proc_record(preview_lines); // 4. hough line
#endif //TIME_TEST

    // 5. filter lines
    Mat result = road_area.clone();
    filterLinesWithAdaptiveROI(result, lines);

#ifdef SHOW
    showImage("result", result, 5, FRAME_WIDTH, FRAME_HEIGHT);
    waitKey(0);
#endif // SHOW

#ifdef TIME_TEST
    tl.proc_record(result); // 5. filter lines
    tl.total_record(frame.getMat(), result); // 6. total
#endif // TIME_TEST
#ifdef VIDEO_SAVE
    putText(result, cv::format("%f", ((double) white / (DEFAULT_ROI_HEIGHT * DEFAULT_ROI_WIDTH) * 100)), Point(50, 50),
            0, 2,
            Scalar(0, 0, 255), 2);
    vw.writeFrame(result, 3);
#endif //VIDEO_SAVE
}

void videoHandler(const string &file_name) {
    VideoCapture video(file_name);

    if (!video.isOpened()) {
        cout << "Can't open video\n";
        return;
    }

    roi = ROI();
    adaptiveThresh = AdaptiveThresh();

    Mat frame;

    while (true) {
        video >> frame;

        if (frame.empty()) {
            break;
        }
#ifdef TIME_TEST
        tl.prev_img = frame.clone(); // 처리 전 원본 사진 저장
#endif //TIME_TEST
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
#ifdef TIME_TEST
        tl = TimeLapse(6);
        tl.set_tc(1);
//        tl.set_tc(stoi(argv[1]));
#endif // TIME_TEST
#ifdef VIDEO_SAVE
        auto pos = file_name.rfind('.');
        string save_path = "../result/video/" + file_name.substr(pos - 2, 2) + ".mp4";
        vw = OneVideoWriter(save_path, DEFAULT_ROI_WIDTH, DEFAULT_ROI_HEIGHT, 2, 2, 4);
#endif //VIDEO_SAVE
#ifdef DETECTION_RATE
        for (int i = 0; i < 3; i++) {
            detected[i] = 0;
        }
        frame_cnt = 0;
#endif //DETECTION_RATE
        videoHandler(file_name);

#ifdef TIME_TEST
        tl.print_info_all();
#endif //TIME_TEST
#ifdef DETECTION_RATE
        for (int i = 0; i < 3; i++) {
            cout << detected[i] << ',';
        }
        cout << frame_cnt << '\n';
#endif //DETECTION_RATE
//        cout << '\n';
    }
    return 0;
}
