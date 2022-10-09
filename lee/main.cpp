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
    int w = 640 / 2;
    if (angle_threshold <= increse_rate && !(line[2] < (w / 2) && line[0] > (w / 2)) && !(line[2] > (w / 2) && line[0] < (w / 2)))
        return true;

    return false;
}

void info_mat(Mat mat)
{
    cout << "width : " << mat.cols << endl;
    cout << "height : " << mat.rows << endl;
    cout << "channels : " << mat.channels() << endl;
}

int main()
{
    VideoCapture cap;
    int w, h;
    Mat frame, resize_frame, gray_frame, mask;
    vector<Point> pts;
    TickMeter tm;
    Mat binarization, edge, dst, closed_edge;
    vector<Vec4i> lines, filtered_lines;
    double fps;
    string filepath = "./src/video/";
    string filename = "복잡한 노면 + 밤길 + 바로 앞에 차.avi";

    cap.open(filepath + filename, CAP_ANY);

    if (!cap.isOpened())
    {
        cerr << " img read failed!" << endl;
        return -1;
    }

    // w = cap.get(CAP_PROP_FRAME_WIDTH);
    // h = cap.get(CAP_PROP_FRAME_HEIGHT);
    w = 640;
    h = 360;

    fps = cap.get(CAP_PROP_FPS);

    int delay = cvRound(1000 / fps);

    pts.push_back(Point(120, 300));
    pts.push_back(Point(310, 180));
    pts.push_back(Point(330, 180));
    pts.push_back(Point(520, 300));

    mask = Mat::zeros(h, w, CV_8UC1);
    fillPoly(mask, pts, Scalar(255), LINE_AA);
    mask = ~mask;

    while (true)
    {
        tm.reset();
        tm.start();

        cap >> frame;
        if (frame.empty())
            break;

        resize(frame, resize_frame, Size(w, h));
        cvtColor(resize_frame, gray_frame, COLOR_BGR2GRAY); // 3채널 -> 1채널
        info_mat(gray_frame);
        adaptiveThreshold(gray_frame, binarization, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 3, 3); // 이진화
        binarization.setTo(Scalar(0), mask);                                                                   // 마스킹

        Canny(binarization, edge, 50, 150, 7); // 에지 검출
        // morphologyEx(edge, closed_edge, MORPH_OPEN, Mat());         // 모폴로지 닫기 연산
        HoughLinesP(edge, lines, 1, CV_PI / 180, 30, 10, 5); // 직선 검출

        filtered_lines.clear();

        for (Vec4i l : lines)
        {
            if (angle_filter(l, 0.5))
            {
                filtered_lines.push_back(l);
            }
        }

        dst = resize_frame.clone();

        for (Vec4i l : filtered_lines)
        {
            line(dst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, LINE_AA);
        }

        line(dst, Point(320, 0), Point(320, 360), Scalar(0, 255, 0), 2, LINE_AA);

        imshow("original", resize_frame);
        imshow("dst", dst);
        imshow("binarization", binarization);
        // imshow("closed_edge", closed_edge);

        resizeWindow("original", w, h);
        resizeWindow("dst", w, h);
        resizeWindow("binarization", w, h);
        // resizeWindow("closed_edge", w, h);

        moveWindow("dst", w, 0);
        moveWindow("binarization", 0, h);
        // moveWindow("closed_edge", w, h);

        tm.stop();
        cout << "Image processing took " << tm.getTimeMilli() << "ms." << endl;

        if (waitKey(delay) > 0)
        {
            imwrite("tmp.png", frame);
            waitKey();
        }
    }

    destroyAllWindows();

    return 0;
}