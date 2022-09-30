#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

typedef Vec4i Line;

bool angle_filter(Line line, double angle_threshold)
{
    double dx = abs(line[2] - line[0]);
    double dy = abs(line[3] - line[1]);
    double increse_rate = dy / dx;

    cout << increse_rate << endl;

    if (angle_threshold <= increse_rate)
        return true;

    return false;
}

int main()
{
    VideoCapture cap;
    int delay = 30;
    int w, h;
    Mat frame, masked, mask;
    vector<Point> pts;
    TickMeter tm;
    Mat binarization, grayscale_frame, edge, dst, closed_edge;
    vector<Vec4i> lines, filtered_lines;
    int total_frame = 1107;
    double fps;
    string filepath = "./src/video/";
    string filename = "frames/0.png";

    pts.push_back(Point(50, 270));
    pts.push_back(Point(220, 160));
    pts.push_back(Point(360, 160));
    pts.push_back(Point(480, 270));

    cap.open(filepath + filename, CAP_ANY);

    // w = cap.get(CAP_PROP_FRAME_WIDTH);
    // h = cap.get(CAP_PROP_FRAME_HEIGHT);
    fps = cap.get(CAP_PROP_FPS);
    w = 1280;
    h = 720;

    int delay = cvRound(1000 / fps);

    mask = Mat::zeros(h, w, CV_8U);
    fillPoly(mask, pts, Scalar(255), LINE_AA);
    mask = ~mask;

    for (int i = 1; i < total_frame; i++)
    {
        tm.reset();
        tm.start();
        // cap.open("./src/video/content/frames/" + to_string(i) + ".png", CAP_ANY);

        if (!cap.isOpened())
        {
            cerr << i << " img read failed!" << endl;
            return -1;
        }

        cap >> frame;
        if (frame.empty())
            break;

        cvtColor(frame, masked, COLOR_BGR2GRAY);                                                           // 3채널 -> 1채널
        adaptiveThreshold(masked, binarization, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 3); // 이진화
        binarization.setTo(Scalar(0, 0, 0), mask);                                                         // 마스킹

        Canny(binarization, edge, 50, 150);                         // 에지 검출
        morphologyEx(edge, closed_edge, MORPH_CLOSE, Mat());        // 모폴로지 닫기 연산
        HoughLinesP(closed_edge, lines, 1, CV_PI / 180, 30, 10, 5); // 직선 검출

        filtered_lines.clear();

        for (Vec4i l : lines)
        {
            if (angle_filter(l, 0.5))
            {
                filtered_lines.push_back(l);
            }
        }

        dst = frame.clone();

        for (Vec4i l : filtered_lines)
        {
            line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, LINE_AA);
        }

        imshow("original", frame);
        imshow("dst", dst);
        imshow("binarization", binarization);
        imshow("closed_edge", closed_edge);

        resizeWindow("original", w, h);
        resizeWindow("dst", w, h);
        resizeWindow("binarization", w, h);
        resizeWindow("closed_edge", w, h);

        moveWindow("dst", w, 0);
        moveWindow("binarization", 0, h);
        moveWindow("closed_edge", w, h);

        tm.stop();
        cout << "Image processing took " << tm.getTimeMilli() << "ms." << endl;

        if (waitKey(delay) == 27)
            break;
    }

    destroyAllWindows();

    return 0;
}