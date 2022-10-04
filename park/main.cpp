#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


const string SRC_PREFIX = "../video/";

vector<Point> polygon(4);
Mat sampleImage;

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
//    std::vector<Point> polygon = {
//            Point(0, row),
//            Point(col / 5 * 2, row / 2),
//            Point(col / 4 * 3, row / 2),
//            Point(col, row)
//    };
//    std::vector<Point> polygon = {
//            Point(50, 270),
//            Point(220, 160),
//            Point(360, 160),
//            Point(480, 270)
//    };
//    polygon = {
//            Point(50, 270),
//            Point(220, 160),
//            Point(360, 160),
//            Point(480, 270)
//    };

    Mat roi = Mat::zeros(row, col, CV_8U);
    fillConvexPoly(roi, polygon, 255);
    bitwise_and(frame, roi, result);
}

void onMouse(int event, int x, int y, int flag, void *userdata) {
    static int cnt = 0;
    if (event == EVENT_LBUTTONDOWN) {
        if (cnt == 4) return;
        cout << x << ' ' << y << '\n';
        polygon[cnt++] = Point(x, y);

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
//    showImage("edge", edge);
    std::vector<Vec4i> lines;
    HoughLinesP(edge, lines, 1, CV_PI / 180, 0, 100, 200);
    drawLines(result, lines);
}

void test(InputArray frame) {
    Mat grayscaled, roi;
    Mat result = frame.getMat().clone();
//    showImage("img", frame);
    thresholdDefault(frame, grayscaled);
//    showImage("grayscaled", grayscaled);
    applyStaticROI(grayscaled, roi);
//    showImage("roi", roi);
    houghLineSegments(roi, result);
//    showImage("result", result);
//    destroyAllWindows();
}

void videoHandler(const string &file_name) {
    VideoCapture capture(file_name);

    Mat frame, resized_frame;

    if (!capture.isOpened()) {
        cout << "Can't open video\n";
        return;
    }
    TickMeter tm;
    tm.reset();
    vector<double> time_record;
    while (true) {
        tm.reset();
        tm.start();
        capture >> frame;

        if (frame.empty()) {
            break;
        }
        resize(frame, resized_frame, Size(640, 480));

        test(frame);
        tm.stop();
        time_record.push_back(tm.getTimeMilli());
    }
    cout << "min: " << *std::min_element(time_record.begin(), time_record.end()) << "ms\n";
    cout << "max: " << *std::max_element(time_record.begin(), time_record.end()) << "ms\n";
    double total = 0;
    for (int i = 1; i < time_record.size(); i++) {
        total += time_record[i];
    }
    cout << total / (double) (time_record.size() - 1) << "ms\n";

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
    glob(SRC_PREFIX + "bright.avi", file_list);

    if (file_list.empty()) {
        cout << "can't find image list\n";
        return 1;
    }
//    VideoCapture cap(file_list[0]);
//    cap >> sampleImage;
//    cout << sampleImage.cols << ' ' << sampleImage.rows;

//    setPolygon();
//    cap.release();
    polygon = {{839 / 3,  637 / 3},
               {554 / 3,  854 / 3},
               {1278 / 3, 816 / 3},
               {1025 / 3, 623 / 3}};
    for (string &file_name: file_list) {
//        imageHandler(file_name);
        videoHandler(file_name);
        break;
    }

    return 0;
}
