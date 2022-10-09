#include <iostream>
#include <algorithm>
#include "opencv2/opencv.hpp"
#include "header/timeRecord.hpp"

using namespace std;
using namespace cv;


const string SRC_PREFIX = "../video/";

vector<Point> roi_polygon(4);
Mat sampleImage;    // to draw roi polygon

timeLapse tl = timeLapse({"thresholding", "roi", "canny", "hough line", "draw line"});

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
//    std::vector<Point> roi_polygon = {
//            Point(0, row),
//            Point(col / 5 * 2, row / 2),
//            Point(col / 4 * 3, row / 2),
//            Point(col, row)
//    };
//    std::vector<Point> roi_polygon = {
//            Point(50, 270),
//            Point(220, 160),
//            Point(360, 160),
//            Point(480, 270)
//    };
//    roi_polygon = {
//            Point(50, 270),
//            Point(220, 160),
//            Point(360, 160),
//            Point(480, 270)
//    };

    Mat roi = Mat::zeros(row, col, CV_8U);
    fillPoly(roi, roi_polygon, 255);
    bitwise_and(frame, roi, result);
}

void onMouse(int event, int x, int y, int flag, void *userdata) {
    static int cnt = 0;
    if (event == EVENT_LBUTTONDOWN) {
        if (cnt == 4) return;
        cout << x << ' ' << y << '\n';
        roi_polygon[cnt++] = Point(x, y);

        circle(sampleImage, Point(x, y), 3, Scalar(0, 0, 255), -1);
        imshow("sample", sampleImage);
    }
}

// 임시
void setPolygon() {
    if (sampleImage.empty()) {
        cout << "Can't open sample image\n";
        return;
    }
    namedWindow("sample");
    moveWindow("sample", 0, 0);
    setMouseCallback("sample", onMouse);
    imshow("sample", sampleImage);
    waitKey();
    destroyAllWindows();
}

// 차선 검출 단계
void drawLines(InputOutputArray frame, const std::vector<Vec4i> &lines) {
    int center = 290;
    Scalar color(255, 0, 0);
    double left_max = 0, right_max = 0;
    std::vector<std::pair<Point, Point>> lane(2, {{0, 0},
                                                  {0, 0}});
    for (Vec4i pts: lines) {
        Point p1(pts[0], pts[1]), p2(pts[2], pts[3]);
        // 직선 필터링... (임시)
        double m = getInclination(p1, p2);
        if (abs(m) < 0.1) {
            line(frame, p1, p2, Scalar(0, 0, 255), 1, 8);
            continue;
        }
        if (m > 0) {
            if (m > left_max) {
                left_max = m;
                lane[0] = {p1, p2};
            }
        } else {
            if (m < right_max) {
                right_max = m;
                lane[1] = {p1, p2};
            }
        }

        line(frame, p1, p2, Scalar(0, 255, 0), 1, 8);
    }
    for (int i = 0; i < 2; i++) {
        line(frame, lane[i].first, lane[i].second, color, 3, 8);
    }
}

void houghLineSegments(InputArray frame, InputOutputArray result) {
    Mat edge;
    Canny(frame, edge, 50, 150);
    tl.setCheckpoint(3);
//    showImage("edge", edge);
    std::vector<Vec4i> lines;
    HoughLinesP(edge, lines, 1, CV_PI / 180, 0, 100, 200);
    tl.setCheckpoint(4);
    drawLines(result, lines);
}

void test(InputArray frame) {
    Mat grayscaled, roi;
    Mat result = frame.getMat().clone();
//    timeLapse tl = timeLapse({"thresholding", "roi", "hough line"});

    tl.setCheckpoint(0);
    thresholdDefault(frame, grayscaled);
    tl.setCheckpoint(1);

//    showImage("grayscaled", grayscaled);
    applyStaticROI(grayscaled, roi);
    tl.setCheckpoint(2);
//    showImage("roi", roi);
    houghLineSegments(roi, result);
    tl.stop(4);
//    showImage("result", result);
//    destroyAllWindows();
}

void videoHandler(const string &file_name) {
    VideoCapture video(file_name);

    if (!video.isOpened()) {
        cout << "Can't open video\n";
        return;
    }

    Mat frame, resized_frame;
    double fps = video.get(CAP_PROP_FPS);
    int delay = cvRound(1000 / fps);
    int idx = 0;

    while (true) {
        video >> frame;

        if (frame.empty()) {
            break;
        }
        resize(frame, resized_frame, Size(640, 480));

        test(frame);
        waitKey(delay);
//        cout << idx++ << ' ';
//        if (idx == 10) break;
    }
//    cout << '\n';

    video.release();
}

void imageHandler(const string &file_name) {
    Mat frame = imread(file_name);
    if (frame.empty()) {
        cout << "Can't open image\n";
        return;
    }

    test(frame);
    destroyAllWindows();
}

int main() {
    vector<string> file_list;
    glob(SRC_PREFIX + "2.avi", file_list);

    if (file_list.empty()) {
        cout << "can't find image list\n";
        return 1;
    }
//    VideoCapture cap(file_list[0]);
//    cap >> sampleImage;

//    setPolygon();
//    cap.release();
    roi_polygon = {{275, 195},
                   {187, 284},
                   {461, 284},
                   {342, 195}};
    for (string &file_name: file_list) {
//        imageHandler(file_name);
        videoHandler(file_name);
        tl.print();
    }

    return 0;
}
